#include "Syntaxes/Exprs/UserUnaryExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Semas/Exprs/UserUnaryExprSema.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "OpResolution.hpp"

namespace Ace
{
    UserUnaryExprSyntax::UserUnaryExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr,
        const SrcLocation& opSrcLocation,
        const Op op
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_OpSrcLocation{ opSrcLocation },
        m_Op{ op }
    {
    }

    auto UserUnaryExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserUnaryExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto UserUnaryExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const UserUnaryExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        auto* const opSymbol = diagnostics.Collect(ResolveUnaryOpSymbol(
            m_OpSrcLocation,
            GetScope(),
            exprSema->GetTypeInfo().Symbol,
            m_Op
        )).value_or(
            GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed
        {
            std::make_shared<const UserUnaryExprSema>(
                GetSrcLocation(),
                exprSema,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserUnaryExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
