#include "Syntaxes/Exprs/DerefAsExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Semas/Exprs/DerefAsExprSema.hpp"

namespace Ace
{
    DerefAsExprSyntax::DerefAsExprSyntax(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto DerefAsExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefAsExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto DerefAsExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const DerefAsExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ITypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed
        {
            std::make_shared<const DerefAsExprSema>(
                GetSrcLocation(),
                exprSema,
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto DerefAsExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
