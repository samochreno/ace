#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <chrono>

#include "LLVM.hpp"
#include "C.hpp"
#include "Scope.hpp"
#include "Symbol/All.hpp"
#include "BoundNode/All.hpp"
#include "ExpressionDropData.hpp"
#include "Emittable.hpp"

namespace Ace
{
    struct BlockBuilder
    {
        BlockBuilder(
            llvm::LLVMContext& t_context,
            llvm::Function* const t_function
        ) : BlockBuilder{ llvm::BasicBlock::Create(t_context, "", t_function) }
        {
        }

        BlockBuilder(
            llvm::BasicBlock* t_block
        ) : Block{ t_block },
            Builder{ llvm::IRBuilder<>{ Block } }
        {
        }

        llvm::BasicBlock* Block;
        llvm::IRBuilder<>  Builder;
    };

    class Emitter
    {
    public:
        struct Result
        {
            struct DurationInfo
            {
                std::chrono::nanoseconds Analyses{};
                std::chrono::nanoseconds LLC{};
                std::chrono::nanoseconds Clang{};
            } Durations{};
        };

        class LabelBlockMap
        {
        public:
            LabelBlockMap(Emitter& t_emitter)
                : m_Emitter{ t_emitter }
            {
            }

            auto GetOrCreateAt(const Symbol::Label* const t_labelSymbol) -> llvm::BasicBlock*
            {
                auto foundIt = m_Map.find(t_labelSymbol);
                if (foundIt != end(m_Map))
                    return foundIt->second;

                auto block = llvm::BasicBlock::Create(
                    m_Emitter.GetContext(),
                    "",
                    m_Emitter.GetFunction()
                );

                m_Map[t_labelSymbol] = block;
                return block;
            }

            auto Clear() -> void
            {
                m_Map.clear();
            }

        private:
            Emitter& m_Emitter;
            std::unordered_map<const Symbol::Label*, llvm::BasicBlock*> m_Map{};
        };

        struct LocalVariableSymbolStatementIndexPair
        {
            Symbol::Variable::Local* LocalVariableSymbol{};
            size_t StatementIndex{};
        };

        Emitter(const std::string& t_packageName);
        ~Emitter();

        auto SetASTs(
            const std::vector<std::shared_ptr<const BoundNode::Module>>& t_asts
        ) -> void { m_ASTs = t_asts; }

        auto Emit() -> Result;

        auto EmitFunctionBodyStatements(
            const std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>& t_statements
        ) -> void;
        auto EmitLoadArgument(
            const size_t& t_index, 
            llvm::Type* const t_type
        ) const -> llvm::Value*;
        auto EmitCopy(
            llvm::Value* const t_lhsValue, 
            llvm::Value* const t_rhsValue, 
            Symbol::Type::IBase* const t_typeSymbol
        ) -> void;
        auto EmitDrop(const ExpressionDropData& t_dropData) -> void;
        auto EmitDropTemporaries(
            const std::vector<ExpressionDropData>& t_temporaries
        ) -> void;
        auto EmitDropLocalVariablesBeforeStatement(
            const BoundNode::Statement::IBase* const t_statement
        ) -> void;
        auto EmitDropArguments() -> void;

        auto GetContext() const -> llvm::LLVMContext& { return *m_Context.get(); }
        auto GetModule() const -> llvm::Module& { return *m_Module.get(); }
        auto GetC() const -> const C& { return m_C; }

        auto GetIRType(const Symbol::Type::IBase* const t_typeSymbol) const -> llvm::Type*;

        auto GetStaticVariableMap() const -> const std::unordered_map<const Symbol::Variable::Normal::Static*, llvm::Constant*>& { return m_StaticVariableMap; }
        auto GetFunctionMap() const -> const std::unordered_map<const Symbol::Function*, llvm::FunctionCallee>& { return m_FunctionMap; }

        auto GetLocalVariableMap() const -> const std::unordered_map<const Symbol::Variable::IBase*, llvm::Value*>& { return m_LocalVariableMap; }
        auto GetLabelBlockMap() -> LabelBlockMap& { return m_LabelBlockMap; }
        auto GetFunction() const -> llvm::Function* { return m_Function; }

        auto GetBlockBuilder() -> BlockBuilder& { return *m_BlockBuilder.get(); }
        auto SetBlockBuilder(
            std::unique_ptr<BlockBuilder>&& t_value
        ) -> void { m_BlockBuilder = std::move(t_value); }

    private:
        auto EmitTypes(
            const std::vector<Symbol::Type::IBase*>& t_typeSymbols
        ) -> void;
        auto EmitGlue(
            const std::vector<Symbol::Type::Struct*>& t_structSymbols,
            const std::function<Symbol::Function*(Symbol::Type::Struct* const)> t_defineSymbols,
            const std::function<bool(Symbol::Type::Struct* const)> t_isBodyTrivial,
            const std::function<std::shared_ptr<const IEmittable<void>>(Symbol::Function* const, Symbol::Type::Struct* const)> t_createTrivialBody,
            const std::function<std::shared_ptr<const IEmittable<void>>(Symbol::Function* const, Symbol::Type::Struct* const)> t_createBody
        ) -> void;
        auto EmitNativeTypes() -> void;
        auto EmitStructTypes(
            const std::vector<Symbol::Type::Struct*>& t_structSymbols
        ) -> void;
        auto EmitStaticVariables(
            const std::vector<Symbol::Variable::Normal::Static*>& t_variableSymbols
        ) -> void;
        auto EmitFunctions(
            const std::vector<Symbol::Function*>& t_functionSymbols
        ) -> void;

        static auto DefineCopyGlueSymbols(
            Symbol::Type::Struct* const t_structSymbol
        ) -> Symbol::Function*;
        static auto DefineDropGlueSymbols(
            Symbol::Type::Struct* const t_structSymbol
        ) -> Symbol::Function*;

        static auto CreateTrivialCopyGlueBody(
            Symbol::Function* const t_glueSymbol,
            Symbol::Type::Struct* const t_structSymbol
        ) -> std::shared_ptr<const IEmittable<void>>;
        static auto CreateCopyGlueBody(
            Symbol::Function* const t_glueSymbol,
            Symbol::Type::Struct* const t_structSymbol
        ) -> std::shared_ptr<const IEmittable<void>>;
        static auto CreateTrivialDropGlueBody(
            Symbol::Function* const t_glueSymbol,
            Symbol::Type::Struct* const t_structSymbol
        ) -> std::shared_ptr<const IEmittable<void>>;
        static auto CreateDropGlueBody(
            Symbol::Function* const t_glueSymbol,
            Symbol::Type::Struct* const t_structSymbol
        ) -> std::shared_ptr<const IEmittable<void>>;

        std::string m_PackageName{};

        std::vector<std::shared_ptr<const BoundNode::Module>> m_ASTs{};

        std::unique_ptr<llvm::LLVMContext> m_Context{};
        std::unique_ptr<llvm::Module> m_Module{};

        std::unordered_map<const Symbol::Type::IBase*, llvm::Type*> m_TypeMap{};
        std::unordered_map<const Symbol::Variable::Normal::Static*, llvm::Constant*> m_StaticVariableMap{};
        std::unordered_map<const Symbol::Function*, llvm::FunctionCallee> m_FunctionMap{};

        std::unordered_map<const Symbol::Variable::IBase*, llvm::Value*> m_LocalVariableMap{};
        LabelBlockMap m_LabelBlockMap;
        std::unordered_map<const BoundNode::Statement::IBase*, size_t> m_StatementIndexMap{};
        std::unordered_map<const Symbol::Variable::Local*, size_t> m_LocalVariableSymbolStatementIndexMap{};
        std::vector<LocalVariableSymbolStatementIndexPair> m_LocalVariableSymbolStatementIndexPairs{};

        llvm::Function* m_Function{};
        Symbol::Function* m_FunctionSymbol{};
        std::unique_ptr<BlockBuilder> m_BlockBuilder{};

        C m_C{};
    };
}
