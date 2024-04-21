#include "Semas/Exprs/TypeInfoPtrExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropInfo.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    TypeInfoPtrExprSema::TypeInfoPtrExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ISizedTypeSymbol* const typeSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeSymbol{ typeSymbol }
    {
    }

    auto TypeInfoPtrExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("TypeInfoPtrExprSema", [&]()
        {
            logger.Log("m_TypeSymbol", m_TypeSymbol);
        });
    }

    auto TypeInfoPtrExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TypeInfoPtrExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TypeInfoPtrExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const TypeInfoPtrExprSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto TypeInfoPtrExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto TypeInfoPtrExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const TypeInfoPtrExprSema>
    {
        return shared_from_this();
    }

    auto TypeInfoPtrExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto TypeInfoPtrExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_TypeSymbol);
    }

    auto TypeInfoPtrExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        auto* const value = emitter.GetTypeInfo(m_TypeSymbol);

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            emitter.GetPtrType()
        );
        tmps.emplace_back(
            allocaInst,
            GetCompilation()->GetNatives().Ptr.GetSymbol()
        );
        emitter.GetBlock().Builder.CreateStore(value, allocaInst);

        return { allocaInst, tmps };
    }

    auto TypeInfoPtrExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { GetCompilation()->GetNatives().Ptr.GetSymbol(), ValueKind::R };
    }
}
