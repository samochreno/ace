#include "Syntaxes/Exprs/AddressOfExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/AddressOfExprSema.hpp"

namespace Ace
{
    AddressOfExprSyntax::AddressOfExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_Expr{ expr }
    {
    }

    auto AddressOfExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_Expr->GetSrcLocation();
    }

    auto AddressOfExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOfExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto AddressOfExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const AddressOfExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        return Diagnosed
        {
            std::make_shared<const AddressOfExprSema>(
                GetSrcLocation(),
                exprSema
            ),
            std::move(diagnostics),
        };
    }

    auto AddressOfExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
