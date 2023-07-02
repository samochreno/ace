#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <chrono>

#include "LLVM.hpp"
#include "C.hpp"
#include "Scope.hpp"
#include "Symbols/All.hpp"
#include "BoundNodes/All.hpp"
#include "ExprDropData.hpp"
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
            const LabelSymbol* const t_labelSymbol
        ) -> llvm::BasicBlock*;
        auto Clear() -> void;

    private:
        Emitter& m_Emitter;
        std::unordered_map<const LabelSymbol*, llvm::BasicBlock*> m_Map{};
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

        struct LocalVarSymbolStmtIndexPair
        {
            LocalVarSymbol* LocalVarSymbol{};
            size_t StmtIndex{};
        };

        Emitter(const Compilation* const t_compilation);
        ~Emitter();

        auto SetASTs(
            const std::vector<std::shared_ptr<const ModuleBoundNode>>& t_asts
        ) -> void;

        auto Emit() -> Result;

        auto EmitFunctionBodyStmts(
            const std::vector<std::shared_ptr<const IStmtBoundNode>>& t_stmts
        ) -> void;
        auto EmitLoadArg(
            const size_t& t_index, 
            llvm::Type* const t_type
        ) const -> llvm::Value*;
        auto EmitCopy(
            llvm::Value* const t_lhsValue, 
            llvm::Value* const t_rhsValue, 
            ITypeSymbol* const t_typeSymbol
        ) -> void;
        auto EmitDrop(const ExprDropData& t_dropData) -> void;
        auto EmitDropTemporaries(
            const std::vector<ExprDropData>& t_temporaries
        ) -> void;
        auto EmitDropLocalVarsBeforeStmt(
            const IStmtBoundNode* const t_stmt
        ) -> void;
        auto EmitDropArgs() -> void;

        auto GetCompilation() const -> const Compilation*;
        auto GetModule() const -> llvm::Module&;
        auto GetC() const -> const C&;

        auto GetIRType(
            const ITypeSymbol* const t_typeSymbol
        ) const -> llvm::Type*;

        auto GetStaticVarMap() const -> const std::unordered_map<const StaticVarSymbol*, llvm::Constant*>&;
        auto GetFunctionMap() const -> const std::unordered_map<const FunctionSymbol*, llvm::FunctionCallee>&;

        auto GetLocalVarMap() const -> const std::unordered_map<const IVarSymbol*, llvm::Value*>&;
        auto GetLabelBlockMap() -> LabelBlockMap&;
        auto GetFunction() const -> llvm::Function*;

        auto GetBlockBuilder() -> BlockBuilder&;
        auto SetBlockBuilder(
            std::unique_ptr<BlockBuilder>&& t_value
        ) -> void;

    private:
        auto EmitTypes(
            const std::vector<ITypeSymbol*>& t_typeSymbols
        ) -> void;
        auto EmitNativeTypes() -> void;
        auto EmitStructTypes(
            const std::vector<StructTypeSymbol*>& t_structSymbols
        ) -> void;
        auto EmitStaticVars(
            const std::vector<StaticVarSymbol*>& t_variableSymbols
        ) -> void;
        auto EmitFunctions(
            const std::vector<FunctionSymbol*>& t_functionSymbols
        ) -> void;

        const Compilation* m_Compilation{};

        std::vector<std::shared_ptr<const ModuleBoundNode>> m_ASTs{};

        std::unique_ptr<llvm::Module> m_Module{};

        std::unordered_map<const ITypeSymbol*, llvm::Type*> m_TypeMap{};
        std::unordered_map<const StaticVarSymbol*, llvm::Constant*> m_StaticVarMap{};
        std::unordered_map<const FunctionSymbol*, llvm::FunctionCallee> m_FunctionMap{};

        std::unordered_map<const IVarSymbol*, llvm::Value*> m_LocalVarMap{};
        LabelBlockMap m_LabelBlockMap;
        std::unordered_map<const IStmtBoundNode*, size_t> m_StmtIndexMap{};
        std::unordered_map<const LocalVarSymbol*, size_t> m_LocalVarSymbolStmtIndexMap{};
        std::vector<LocalVarSymbolStmtIndexPair> m_LocalVarSymbolStmtIndexPairs{};

        llvm::Function* m_Function{};
        FunctionSymbol* m_FunctionSymbol{};
        std::unique_ptr<BlockBuilder> m_BlockBuilder{};

        C m_C{};
    };
}
