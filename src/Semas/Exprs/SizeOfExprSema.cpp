#include "Semas/Exprs/SizeOfExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropInfo.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    SizeOfExprSema::SizeOfExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeSymbol{ typeSymbol }
    {
    }

    auto SizeOfExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SizeOfExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOfExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const SizeOfExprSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto SizeOfExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto SizeOfExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const SizeOfExprSema>
    {
        return shared_from_this();
    }

    auto SizeOfExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto SizeOfExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_TypeSymbol);
    }

    auto SizeOfExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        auto* const intTypeSymbol =
            GetCompilation()->GetNatives().Int.GetSymbol();

        auto* const intType = emitter.GetType(intTypeSymbol);
        auto* const type = emitter.GetType(m_TypeSymbol);

        auto* const value = llvm::ConstantInt::get(
            intType,
            emitter.GetModule().getDataLayout().getTypeAllocSize(type)
        );

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            intType
        );
        tmps.emplace_back(allocaInst, intTypeSymbol);
        
        emitter.GetBlock().Builder.CreateStore(value, allocaInst);

        return { allocaInst, tmps };
    }

    auto SizeOfExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives().Int.GetSymbol(),
            ValueKind::R
        };
    }
}
