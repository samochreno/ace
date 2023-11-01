#include "Syntaxes/Exprs/MemberAccessExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Semas/Exprs/VarRefs/FieldVarRefExprSema.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    MemberAccessExprSyntax::MemberAccessExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr,
        const SymbolNameSection& name
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_Name{ name }
    {
    }

    auto MemberAccessExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto MemberAccessExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto MemberAccessExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto MemberAccessExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const FieldVarRefExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        auto* const selfTypeSymbol =
            exprSema->GetTypeInfo().Symbol->GetWithoutRef();

        std::optional<FieldVarSymbol*> optFieldSymbol{};
        if (!selfTypeSymbol->IsError())
        {
            optFieldSymbol = diagnostics.Collect(
                GetScope()->ResolveInstanceSymbol<FieldVarSymbol>(
                    selfTypeSymbol,
                    m_Name
                )
            );
        }

        auto* const fieldSymbol = optFieldSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetField()
        );

        return Diagnosed
        {
            std::make_shared<const FieldVarRefExprSema>(
                GetSrcLocation(),
                exprSema,
                fieldSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto MemberAccessExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }

    auto MemberAccessExprSyntax::GetExpr() const -> const IExprSyntax*
    {
        return m_Expr.get();
    }

    auto MemberAccessExprSyntax::GetName() const -> const SymbolNameSection&
    {
        return m_Name;
    }
}
