#include "Syntaxes/Stmts/IfStmtSyntax.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/IfStmtSema.hpp"

namespace Ace
{
    IfStmtSyntax::IfStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IExprSyntax>>& conditions,
        const std::vector<std::shared_ptr<const BlockStmtSyntax>>& blocks
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Conditions{ conditions },
        m_Blocks{ blocks }

    {
    }

    auto IfStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto IfStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto IfStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Conditions)
            .Collect(m_Blocks)
            .Build();
    }

    auto IfStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const IfStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const IExprSema>> conditionSemas{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(conditionSemas),
            [&](const std::shared_ptr<const IExprSyntax>& condition)
            {
                return diagnostics.Collect(condition->CreateExprSema());
            }
        );

        std::vector<std::shared_ptr<const BlockStmtSema>> blockSemas{};
        std::transform(
            begin(m_Blocks),
            end  (m_Blocks),
            back_inserter(blockSemas),
            [&](const std::shared_ptr<const BlockStmtSyntax>& block)
            {
                return diagnostics.Collect(block->CreateSema());
            }
        );

        return Diagnosed
        {
            std::make_shared<const IfStmtSema>(
                GetSrcLocation(),
                GetScope(),
                conditionSemas,
                blockSemas
            ),
            std::move(diagnostics),
        };
    }

    auto IfStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
