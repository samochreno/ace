#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    GroupStmtBoundNode::GroupStmtBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& t_stmts
    ) : m_Scope{ t_scope },
        m_Stmts{ t_stmts }
    {
    }

    auto GroupStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto GroupStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto GroupStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const IStmtBoundNode>& t_stmt)
        {
            return t_stmt->GetOrCreateTypeCheckedStmt({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (!mchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            m_Scope,
            mchCheckedContent.Value
        ));
    }

    auto GroupStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto GroupStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto mchLoweredStmts = TransformMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const IStmtBoundNode>& t_stmt)
        {
            return t_stmt->GetOrCreateLoweredStmt({});
        });

        if (!mchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            m_Scope,
            mchLoweredStmts.Value
        )->GetOrCreateLowered(t_context).Value);
    }

    auto GroupStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto GroupStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
    
    auto GroupStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        return m_Stmts;
    }
}
