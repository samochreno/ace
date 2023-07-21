#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    GroupStmtBoundNode::GroupStmtBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Stmts{ stmts }
    {
    }

    auto GroupStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
        {
            return stmt->GetOrCreateTypeCheckedStmt({
                context.ParentFunctionTypeSymbol
            });
        }));

        if (!mchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            GetSourceLocation(),
            GetScope(),
            mchCheckedContent.Value
        ));
    }

    auto GroupStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto GroupStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto mchLoweredStmts = TransformMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
        {
            return stmt->GetOrCreateLoweredStmt({});
        });

        if (!mchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            GetSourceLocation(),
            GetScope(),
            mchLoweredStmts.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto GroupStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto GroupStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
    
    auto GroupStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        return m_Stmts;
    }
}
