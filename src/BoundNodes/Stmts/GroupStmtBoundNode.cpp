#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    GroupStmtBoundNode::GroupStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Stmts{ stmts }
    {
    }

    auto GroupStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto GroupStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto GroupStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto GroupStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const GroupStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const IStmtBoundNode>> checkedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(checkedStmts),
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                const auto dgnCheckedStmt = stmt->CreateTypeCheckedStmt({
                    context.ParentFunctionTypeSymbol
                });
                diagnostics.Add(dgnCheckedStmt);
                return dgnCheckedStmt.Unwrap();
            }
        );

        if (checkedStmts == m_Stmts)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const GroupStmtBoundNode>(
                GetSrcLocation(),
                GetScope(),
                checkedStmts
            ),
            diagnostics,
        };
    }

    auto GroupStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto GroupStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtBoundNode>
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> loweredStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(loweredStmts),
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                return stmt->CreateLoweredStmt({});
            }
        );

        if (loweredStmts == m_Stmts)
        {
            return shared_from_this();
        }

        return std::make_shared<const GroupStmtBoundNode>(
            GetSrcLocation(),
            GetScope(),
            loweredStmts
        )->CreateLowered({});
    }

    auto GroupStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto GroupStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto GroupStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        std::vector<CFANode> nodes{};
        std::for_each(
            begin(m_Stmts),
            end  (m_Stmts),
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                const auto stmtNodes = stmt->CreateCFANodes();
                nodes.insert(end(nodes), begin(stmtNodes), end(stmtNodes));
            }
        );

        return nodes;
    }
    
    auto GroupStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        return m_Stmts;
    }
}
