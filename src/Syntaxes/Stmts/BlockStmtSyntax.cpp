#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"

namespace Ace
{
    BlockStmtSyntax::BlockStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const std::vector<std::shared_ptr<const IStmtSyntax>>& stmts
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_Stmts{ stmts }
    {
    }

    auto BlockStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BlockStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto BlockStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Stmts).Build();
    }

    auto BlockStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const BlockStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const IStmtSema>> stmtSemas{};
        std::transform(begin(m_Stmts), end(m_Stmts), back_inserter(stmtSemas),
        [&](const std::shared_ptr<const IStmtSyntax>& stmt)
        {
            return diagnostics.Collect(stmt->CreateStmtSema());
        });

        return Diagnosed
        {
            std::make_shared<const BlockStmtSema>(
                GetSrcLocation(),
                m_BodyScope,
                stmtSemas
            ),
            std::move(diagnostics),
        };
    }

    auto BlockStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
