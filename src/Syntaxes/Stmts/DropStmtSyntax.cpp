#include "Syntaxes/Stmts/DropStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Semas/Stmts/DropStmtSema.hpp"

namespace Ace
{
    DropStmtSyntax::DropStmtSyntax(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto DropStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DropStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DropStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Expr)
            .Build();
    }

    auto DropStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const DropStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed
        {
            std::make_shared<const DropStmtSema>(
                GetSrcLocation(),
                typeSymbol,
                exprSema
            ),
            std::move(diagnostics),
        };
    }

    auto DropStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
