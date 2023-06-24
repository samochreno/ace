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
#include "Compilation.hpp"

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

    class LabelBlockMap
    {
    public:
        LabelBlockMap(Emitter& t_emitter);

        auto GetOrCreateAt(
            const Symbol::Label* const t_labelSymbol
        ) -> llvm::BasicBlock*;
        auto Clear() -> void;

    private:
        Emitter& m_Emitter;
        std::unordered_map<const Symbol::Label*, llvm::BasicBlock*> m_Map{};
    };

    class Emitter
    {
    public:
        struct Result
        {
            struct DurationInfo
            {
                std::chrono::nanoseconds IREmitting{};
                std::chrono::nanoseconds Analyses{};
                std::chrono::nanoseconds LLC{};
                std::chrono::nanoseconds Clang{};
            } Durations{};
        };

        struct LocalVariableSymbolStatementIndexPair
        {
            Symbol::Variable::Local* LocalVariableSymbol{};
            size_t StatementIndex{};
        };

        Emitter(const Compilation* const t_compilation);
        ~Emitter();

        auto SetASTs(
            const std::vector<std::shared_ptr<const BoundNode::Module>>& t_asts
        ) -> void;

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

        auto GetCompilation() const -> const Compilation*;
        auto GetModule() const -> llvm::Module&;
        auto GetC() const -> const C&;

        auto GetIRType(
            const Symbol::Type::IBase* const t_typeSymbol
        ) const -> llvm::Type*;

        auto GetStaticVariableMap() const -> const std::unordered_map<const Symbol::Variable::Normal::Static*, llvm::Constant*>&;
        auto GetFunctionMap() const -> const std::unordered_map<const Symbol::Function*, llvm::FunctionCallee>&;

        auto GetLocalVariableMap() const -> const std::unordered_map<const Symbol::Variable::IBase*, llvm::Value*>&;
        auto GetLabelBlockMap() -> LabelBlockMap&;
        auto GetFunction() const -> llvm::Function*;

        auto GetBlockBuilder() -> BlockBuilder&;
        auto SetBlockBuilder(
            std::unique_ptr<BlockBuilder>&& t_value
        ) -> void;

    private:
        auto EmitTypes(
            const std::vector<Symbol::Type::IBase*>& t_typeSymbols
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

        const Compilation* m_Compilation{};

        std::vector<std::shared_ptr<const BoundNode::Module>> m_ASTs{};

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
