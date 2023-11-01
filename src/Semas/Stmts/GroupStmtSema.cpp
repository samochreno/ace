#include "Semas/Stmts/GroupStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    GroupStmtSema::GroupStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IStmtSema>>& stmts
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Stmts{ stmts }
    {
    }

    auto GroupStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto GroupStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto GroupStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const GroupStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const IStmtSema>> checkedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(checkedStmts),
            [&](const std::shared_ptr<const IStmtSema>& stmt)
            {
                return diagnostics.Collect(stmt->CreateTypeCheckedStmt({
                    context.ParentFunctionTypeSymbol
                }));
            }
        );

        if (checkedStmts == m_Stmts)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const GroupStmtSema>(
                GetSrcLocation(),
                GetScope(),
                checkedStmts
            ),
            std::move(diagnostics),
        };
    }

    auto GroupStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto GroupStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtSema>
    {
        std::vector<std::shared_ptr<const IStmtSema>> loweredStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(loweredStmts),
            [&](const std::shared_ptr<const IStmtSema>& stmt)
            {
                return stmt->CreateLoweredStmt({});
            }
        );

        if (loweredStmts == m_Stmts)
        {
            return shared_from_this();
        }

        return std::make_shared<const GroupStmtSema>(
            GetSrcLocation(),
            GetScope(),
            loweredStmts
        )->CreateLowered({});
    }

    auto GroupStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto GroupStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Stmts);
    }

    auto GroupStmtSema::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto GroupStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        std::vector<ControlFlowNode> nodes{};
        std::for_each(begin(m_Stmts), end(m_Stmts),
        [&](const std::shared_ptr<const IStmtSema>& stmt)
        {
            const auto stmtNodes = stmt->CreateControlFlowNodes();
            nodes.insert(end(nodes), begin(stmtNodes), end(stmtNodes));
        });

        return nodes;
    }
    
    auto GroupStmtSema::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtSema>>
    {
        return m_Stmts;
    }
}
