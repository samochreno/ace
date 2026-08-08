#include "Syntaxes/Stmts/VarStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/VarStmtSema.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"

namespace Ace
{
    VarStmtSyntax::VarStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const std::optional<std::shared_ptr<const IExprSyntax>>& optAssignedExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_OptAssignedExpr{ optAssignedExpr }
    {
    }

    auto VarStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto VarStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VarStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Attributes)
            .Collect(m_OptAssignedExpr)
            .Build();
    }

    auto VarStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const VarStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const selfSymbol = DiagnosticBag::CreateNoError().Collect(
            GetScope()->ResolveStaticSymbol<LocalVarSymbol>(m_Name)
        ).value();

        std::optional<std::shared_ptr<const IExprSema>> optAssignedExprSema{};
        if (m_OptAssignedExpr.has_value())
        {
            optAssignedExprSema = diagnostics.Collect(
                m_OptAssignedExpr.value()->CreateExprSema()
            );
        }

        return Diagnosed
        {
            std::make_shared<const VarStmtSema>(
                GetSrcLocation(),
                selfSymbol,
                optAssignedExprSema
            ),
            std::move(diagnostics),
        };
    }

    auto VarStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }

    auto VarStmtSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VarStmtSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto VarStmtSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<LocalVarSymbol>(
                GetSymbolScope(),
                m_Name,
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }
}
