#include "Syntaxes/Stmts/ExitStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/ExitStmtSema.hpp"

namespace Ace
{
    ExitStmtSyntax::ExitStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope }
    {
    }

    auto ExitStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExitStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ExitStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*> 
    {
        return SyntaxChildCollector{}.Build();
    }

    auto ExitStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const ExitStmtSema>>
    {
        return Diagnosed
        {
            std::make_shared<const ExitStmtSema>(GetSrcLocation(), GetScope()),
            DiagnosticBag::Create(),
        };
    }

    auto ExitStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
