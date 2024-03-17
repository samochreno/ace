#include "Syntaxes/Exprs/UserBinaryExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/UserBinaryExprSema.hpp"
#include "OpResolution.hpp"

namespace Ace
{
    UserBinaryExprSyntax::UserBinaryExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr,
        const SrcLocation& opSrcLocation,
        const Op op
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSrcLocation{ opSrcLocation },
        m_Op{ op }
    {
    }

    auto UserBinaryExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr)
            .Build();
    }


    auto UserBinaryExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const UserBinaryExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto lhsExprSema = diagnostics.Collect(
            m_LHSExpr->CreateExprSema()
        );
        const auto rhsExprSema = diagnostics.Collect(
            m_RHSExpr->CreateExprSema()
        );

        const std::vector<TypeInfo> argTypeInfos
        {
            lhsExprSema->GetTypeInfo(),
            rhsExprSema->GetTypeInfo(),
        };

        const std::vector<ITypeSymbol*> typeSymbols
        {
            argTypeInfos.front().Symbol,
            argTypeInfos. back().Symbol,
        };

        auto* const opSymbol = diagnostics.Collect(ResolveBinaryOpSymbol(
            m_OpSrcLocation,
            GetScope(),
            typeSymbols,
            argTypeInfos,
            m_Op
        )).value_or(
            GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed
        {
            std::make_shared<const UserBinaryExprSema>(
                GetSrcLocation(),
                lhsExprSema,
                rhsExprSema,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserBinaryExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
