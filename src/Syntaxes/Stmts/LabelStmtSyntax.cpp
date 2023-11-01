#include "Syntaxes/Stmts/LabelStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/LabelStmtSema.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    LabelStmtSyntax::LabelStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto LabelStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LabelStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto LabelStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const LabelStmtSema>>
    {
        auto* const selfSymbol = DiagnosticBag::CreateNoError().Collect(
            GetScope()->ResolveStaticSymbol<LabelSymbol>(m_Name)
        ).value();

        return Diagnosed
        {
            std::make_shared<const LabelStmtSema>(GetSrcLocation(), selfSymbol),
            DiagnosticBag::Create(),
        };
    }

    auto LabelStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }

    auto LabelStmtSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelStmtSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::BeforeType;
    }

    auto LabelStmtSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<LabelSymbol>(m_Scope, m_Name),
            DiagnosticBag::Create(),
        };
    }

    auto LabelStmtSyntax::GetName() const -> const Ident&
    {
        return m_Name;
    }
}
