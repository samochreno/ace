#include "Syntaxes/Stmts/WhileStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Semas/Stmts/WhileStmtSema.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    WhileStmtSyntax::WhileStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprSyntax>& condition,
        const std::shared_ptr<const BlockStmtSyntax>& block
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition },
        m_Block{ block }
    {
    }

    auto WhileStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto WhileStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto WhileStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Condition)
            .Collect(m_Block)
            .Build();
    }

    auto WhileStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const WhileStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto conditionSema = diagnostics.Collect(
            m_Condition->CreateExprSema()
        );

        const auto blockSema = diagnostics.Collect(m_Block->CreateSema());

        return Diagnosed
        {
            std::make_shared<const WhileStmtSema>(
                GetSrcLocation(),
                GetScope(),
                conditionSema,
                blockSema
            ),
            std::move(diagnostics),
        };
    }

    auto WhileStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
