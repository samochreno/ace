#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <chrono>

#include "LLVM.hpp"
#include "C.hpp"
#include "Scope.hpp"
#include "Symbols/All.hpp"
#include "Semas/All.hpp"
#include "ExprDropInfo.hpp"
#include "Emittable.hpp"
#include "Compilation.hpp"

namespace Ace
{
    struct EmittingBlock
    {
        EmittingBlock(
            llvm::LLVMContext& context,
            llvm::Function* const function
        ) : EmittingBlock{ llvm::BasicBlock::Create(context, "", function) }
        {
        }
        EmittingBlock(
            llvm::BasicBlock* block
        ) : Block{ block },
            Builder{ llvm::IRBuilder<>{ Block } }
        {
        }

        auto IsTerminated() const -> bool;

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
        std::map<const LabelSymbol*, llvm::BasicBlock*> m_Map{};
    };

    struct LocalVarSymbolStmtIndexPair
    {
        LocalVarSymbol* LocalVarSymbol{};
        size_t StmtIndex{};
    };

    struct EmittingResult
    {
        struct DurationInfo
        {
            std::chrono::nanoseconds GlueGeneration{};
            std::chrono::nanoseconds IREmitting{};
            std::chrono::nanoseconds Analyses{};
            std::chrono::nanoseconds LLC{};
            std::chrono::nanoseconds Clang{};
        } Durations{};
    };

    class Emitter
    {
    public:
        Emitter(Compilation* const compilation);
        ~Emitter();

        auto Emit() -> Expected<EmittingResult>;

        template<typename T>
        auto CreateInstantiated(const IGenericSymbol* const symbol) const -> T*
        {
            if (m_FunctionSymbol && m_FunctionSymbol->GetTypeArgs().empty())
            {
                auto* const castedSymbol =
                    dynamic_cast<T*>(const_cast<IGenericSymbol*>(symbol));
                ACE_ASSERT(castedSymbol);
                return castedSymbol;
            }

            std::vector<ITypeSymbol*> typeArgs{};
            if (m_FunctionSymbol)
            {
                typeArgs = m_FunctionSymbol->GetTypeArgs();
            }

            const InstantiationContext context{ typeArgs, std::nullopt };
            return Ace::CreateInstantiated<T>(symbol, context);
        }

        auto EmitFunctionBlockStmts(
            const std::vector<std::shared_ptr<const IStmtSema>>& stmts
        ) -> void;
        auto EmitCall(
            ICallableSymbol* const callableSymbol,
            const std::vector<llvm::Value*>& args
        ) -> llvm::Value*;
        auto EmitLoadArg(
            const size_t index, 
            llvm::Type* const type
        ) -> llvm::Value*;
        auto EmitCopy(
            llvm::Value* const lhsValue, 
            llvm::Value* const rhsValue, 
            ITypeSymbol* const typeSymbol
        ) -> void;
        auto EmitDrop(const ExprDropInfo& info) -> void;
        auto EmitDropTmps(const std::vector<ExprDropInfo>& tmps) -> void;
        auto EmitDropLocalVarsBeforeStmt(const IStmtSema* const stmt) -> void;
        auto EmitDropArgs() -> void;
        auto EmitString(const std::string_view string) -> llvm::Value*;
        auto EmitPrintf(llvm::Value* const message) -> void;
        auto EmitPrintf(const std::vector<llvm::Value*>& args) -> void;
        auto EmitPrint    (const std::string_view string) -> void;
        auto EmitPrintLine(const std::string_view string) -> void;

        auto GetCompilation() const -> Compilation*;
        auto GetContext() const -> const llvm::LLVMContext&;
        auto GetContext()       ->       llvm::LLVMContext&;
        auto GetModule() const -> const llvm::Module&;
        auto GetModule()       ->       llvm::Module&;
        auto GetC() const -> const C&;

        auto GetType(const ITypeSymbol* const symbol) const -> llvm::Type*;
        auto GetTypeInfo(
            const ITypeSymbol* const symbol
        ) const -> llvm::Constant*;
        auto GetVtbl(
            const ITypeSymbol* const traitSymbol,
            const ITypeSymbol* const typeSymbol
        ) const -> llvm::Constant*;

        auto GetGlobalVar(
            const GlobalVarSymbol* const symbol
        ) const -> llvm::Constant*;
        auto GetFunction(
            const FunctionSymbol* const symbol
        ) const -> llvm::Function*;

        auto GetLocalVar(const IVarSymbol* const symbol) const -> llvm::Value*;
        auto GetLabelBlockMap() -> LabelBlockMap&;
        auto GetFunction() const -> llvm::Function*;

        auto GetBlock() -> EmittingBlock&;
        auto SetBlock(std::unique_ptr<EmittingBlock> value) -> void;

        auto GetPtrType() const -> llvm::PointerType*;
        auto GetTypeInfoType() -> llvm::StructType*;
        auto GetDropGlueType() const -> llvm::FunctionType*;

    private:
        struct FunctionHeader
        {
            llvm::Function* Function{};
            FunctionSymbol* Symbol{};
            llvm::BasicBlock* Block{};
        };

        struct TypeInfoHeader
        {
            llvm::StructType* Type{};
            llvm::GlobalVariable* Var{};
            llvm::Constant* DropGluePtr{};
            std::vector<ITypeSymbol*> TypeSymbols{};
            std::vector<llvm::Constant*> Vtbls{};
        };

        struct TypeInfoHeaderInfo
        {
            llvm::Constant* DropGluePtr{};
            std::vector<ITypeSymbol*> TypeSymbols{};
            std::vector<llvm::Constant*> Vtbls{};
        };

        auto EmitGlobalVar(
            const std::string& name,
            llvm::Type* const type,
            const bool isConstant,
            llvm::Constant* const initializer = nullptr
        ) -> llvm::GlobalVariable*;
        auto EmitTypeInfos(const std::vector<ITypeSymbol*>& symbols) -> void;
        auto EmitTypeInfoHeader(ITypeSymbol* const symbol) -> std::optional<TypeInfoHeader>;
        auto EmitConcreteTypeInfoHeaderInfo(
            IConcreteTypeSymbol* const symbol
        ) -> TypeInfoHeaderInfo;
        auto EmitTraitTypeInfoHeaderInfo(
            TraitTypeSymbol* const symbol
        ) -> TypeInfoHeaderInfo;
        auto EmitTypeInfoBody(const TypeInfoHeader& header) -> void;
        auto EmitVtbls(
            const std::vector<TraitImplSymbol*>& implSymbols
        ) -> void;
        auto EmitVtbl(TraitImplSymbol* const implSymbol) -> void;
        auto EmitNativeTypes() -> void;
        auto EmitStructTypes(
            const std::vector<StructTypeSymbol*>& symbols
        ) -> void;
        auto EmitGlobalVars(
            const std::vector<GlobalVarSymbol*>& symbols
        ) -> void;
        auto EmitFunctions(
            const std::vector<FunctionSymbol*>& symbols
        ) -> void;
        auto EmitFunctionHeader(FunctionSymbol* const symbol) -> FunctionHeader;
        auto EmitFunctionBlock(const FunctionHeader& header) -> void;
        auto EmitStaticCall(
            FunctionSymbol* const functionSymbol,
            const std::vector<llvm::Value*>& args
        ) -> llvm::Value*;
        auto EmitDynCall(
            PrototypeSymbol* const prototypeSymbol,
            const std::vector<llvm::Value*>& args
        ) -> llvm::Value*;

        auto ClearFunctionData() -> void;

        auto GetDropGluePtr(
            ITypeSymbol* const typeSymbol
        ) const -> llvm::Constant*;

        Compilation* m_Compilation{};

        llvm::LLVMContext m_Context{};
        std::unique_ptr<llvm::Module> m_Module{};

        llvm::StructType* m_TypeInfoType{};

        std::map<const ITypeSymbol*, llvm::Constant*> m_TypeInfoMap{};
        std::map<const TraitTypeSymbol*, std::map<const ITypeSymbol*, llvm::Constant*>> m_VtblMap{};
        std::map<const ITypeSymbol*, llvm::Type*> m_TypeMap{};
        std::map<const GlobalVarSymbol*, llvm::Constant*> m_GlobalVarMap{};
        std::map<const FunctionSymbol*, llvm::Function*> m_FunctionMap{};

        std::map<const IVarSymbol*, llvm::Value*> m_LocalVarMap{};
        LabelBlockMap m_LabelBlockMap;
        std::map<const IStmtSema*, size_t> m_StmtIndexMap{};
        std::map<const LocalVarSymbol*, size_t> m_LocalVarSymbolStmtIndexMap{};
        std::vector<LocalVarSymbolStmtIndexPair> m_LocalVarSymbolStmtIndexPairs{};

        llvm::Function* m_Function{};
        FunctionSymbol* m_FunctionSymbol{};
        std::unique_ptr<EmittingBlock> m_Block{};

        C m_C{};
    };
}
