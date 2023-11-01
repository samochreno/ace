#include "Syntaxes/Exprs/CastExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    CastExprSyntax::CastExprSyntax(
        const SrcLocation& srcLocation,
        const TypeName& typeName,
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto CastExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CastExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto CastExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto CastExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ITypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        const auto convertedExprSema = diagnostics.Collect(
            CreateExplicitlyConverted(
                exprSema,
                TypeInfo{ typeSymbol, ValueKind::R }
            )
        );

        return Diagnosed{ convertedExprSema, std::move(diagnostics) };
    }

    auto CastExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
