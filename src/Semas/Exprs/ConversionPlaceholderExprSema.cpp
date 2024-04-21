#include "Semas/Exprs/ConversionPlaceholderExprSema.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    ConversionPlaceholderExprSema::ConversionPlaceholderExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& typeInfo
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeInfo{ typeInfo }
    {
    }

    auto ConversionPlaceholderExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("ConversionPlaceholderExprSema", [&]()
        {
            logger.Log("m_TypeInfo", m_TypeInfo);
        });
    }

    auto ConversionPlaceholderExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ConversionPlaceholderExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ConversionPlaceholderExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ConversionPlaceholderExprSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto ConversionPlaceholderExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto ConversionPlaceholderExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ConversionPlaceholderExprSema>
    {
        return shared_from_this();
    }

    auto ConversionPlaceholderExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto ConversionPlaceholderExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{};
    }

    auto ConversionPlaceholderExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprSema::GetTypeInfo() const -> TypeInfo
    {
        return m_TypeInfo;
    }
}
