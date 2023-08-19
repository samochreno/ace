#pragma once

#include <memory>
#include <vector>
#include <string_view>
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
            llvm::LLVMContext& context,
            llvm::Function* const function
        ) : BlockBuilder{ llvm::BasicBlock::Create(context, "", function) }
        {
        }
        BlockBuilder(
            llvm::BasicBlock* block
        ) : Block{ block },
            Builder{ llvm::IRBuilder<>{ Block } }
        {
        }

        llvm::BasicBlock* Block;
        llvm::IRBuilder<>  Builder;
    };

    class LabelBlockMap
    {
    public:
        LabelBlockMap(Emitter& emitter);

        auto GetOrCreateAt(
            const LabelSymbol* const labelSymbol
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

        Emitter(Compilation* const compilation);
        ~Emitter();

        auto SetASTs(
            const std::vector<std::shared_ptr<const ModuleBoundNode>>& asts
        ) -> void;

        auto Emit() -> Result;

        auto EmitFunctionBodyStmts(
            const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
        ) -> void;
        auto EmitLoadArg(
            const size_t index, 
            llvm::Type* const type
        ) const -> llvm::Value*;
        auto EmitCopy(
            llvm::Value* const lhsValue, 
            llvm::Value* const rhsValue, 
            ISizedTypeSymbol* const typeSymbol
        ) -> void;
        auto EmitDrop(const ExprDropData& dropData) -> void;
        auto EmitDropTmps(
            const std::vector<ExprDropData>& tmps
        ) -> void;
        auto EmitDropLocalVarsBeforeStmt(
            const IStmtBoundNode* const stmt
        ) -> void;
        auto EmitDropArgs() -> void;
        auto EmitString(const std::string_view string) -> llvm::Value*;
        auto EmitPrintf(llvm::Value* const message) -> void;
        auto EmitPrintf(const std::vector<llvm::Value*>& args) -> void;

        auto GetCompilation() const -> Compilation*;
        auto GetContext() const -> const llvm::LLVMContext&;
        auto GetContext()       ->       llvm::LLVMContext&;
        auto GetModule() const -> const llvm::Module&;
        auto GetModule()       ->       llvm::Module&;
        auto GetC() const -> const C&;

        auto GetIRType(
            const ITypeSymbol* const typeSymbol
        ) const -> llvm::Type*;

        auto GetStaticVarMap() const -> const std::unordered_map<const StaticVarSymbol*, llvm::Constant*>&;
        auto GetFunctionMap() const -> const std::unordered_map<const FunctionSymbol*, llvm::FunctionCallee>&;

        auto GetLocalVarMap() const -> const std::unordered_map<const IVarSymbol*, llvm::Value*>&;
        auto GetLabelBlockMap() -> LabelBlockMap&;
        auto GetFunction() const -> llvm::Function*;

        auto GetBlockBuilder() -> BlockBuilder&;
        auto SetBlockBuilder(
            std::unique_ptr<BlockBuilder>&& value
        ) -> void;

    private:
        auto EmitTypes(
            const std::vector<ITypeSymbol*>& typeSymbols
        ) -> void;
        auto EmitNativeTypes() -> void;
        auto EmitStructTypes(
            const std::vector<StructTypeSymbol*>& structSymbols
        ) -> void;
        auto EmitStaticVars(
            const std::vector<StaticVarSymbol*>& varSymbols
        ) -> void;
        auto EmitFunctions(
            const std::vector<FunctionSymbol*>& functionSymbols
        ) -> void;

        Compilation* m_Compilation{};

        std::vector<std::shared_ptr<const ModuleBoundNode>> m_ASTs{};

        llvm::LLVMContext m_Context{};
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
