#include "Syntaxes/Stmts/AssertStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/AssertStmtSema.hpp"

namespace Ace
{
    AssertStmtSyntax::AssertStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprSyntax>& condition
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition }
    {
    }

    auto AssertStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AssertStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto AssertStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Condition).Build();
    }

    auto AssertStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const AssertStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();
        
        const auto conditionSema = diagnostics.Collect(
            m_Condition->CreateExprSema()
        );

        return Diagnosed
        {
            std::make_shared<const AssertStmtSema>(
                GetSrcLocation(),
                conditionSema
            ),
            std::move(diagnostics),
        };
    }

    auto AssertStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
