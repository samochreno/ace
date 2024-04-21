#include "Semas/Exprs/VtblPtrExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
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
    VtblPtrExprSema::VtblPtrExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ISizedTypeSymbol* const typeSymbol,
        ITypeSymbol* const traitSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeSymbol{ typeSymbol },
        m_TraitSymbol{ traitSymbol }
    {
    }

    auto VtblPtrExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("VtblPtrExprSema", [&]()
        {
            logger.Log("m_TypeSymbol", m_TypeSymbol);
            logger.Log("m_TraitSymbol", m_TraitSymbol);
        });
    }

    auto VtblPtrExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto VtblPtrExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VtblPtrExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const VtblPtrExprSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto VtblPtrExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto VtblPtrExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const VtblPtrExprSema>
    {
        return shared_from_this();
    }

    auto VtblPtrExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto VtblPtrExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_TypeSymbol)
            .Collect(m_TraitSymbol);
    }

    auto VtblPtrExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        ACE_ASSERT([&]() -> bool
        {
            auto* const traitSymbol =
                emitter.CreateInstantiated<TraitTypeSymbol>(m_TraitSymbol);

            auto* const typeSymbol =
                emitter.CreateInstantiated<ITypeSymbol>(m_TypeSymbol);

            if (traitSymbol->IsError() || typeSymbol->IsError())
            {
                return true;
            }

            return Scope::CollectImplOfFor(traitSymbol, typeSymbol).has_value();
        }());

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            emitter.GetPtrType()
        );
        tmps.emplace_back(
            allocaInst,
            GetCompilation()->GetNatives().Ptr.GetSymbol()
        );

        emitter.GetBlock().Builder.CreateStore(
            emitter.GetVtbl(m_TraitSymbol, m_TypeSymbol),
            allocaInst
        );

        return { allocaInst, tmps };
    }

    auto VtblPtrExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { GetCompilation()->GetNatives().Ptr.GetSymbol(), ValueKind::R };
    }
}
