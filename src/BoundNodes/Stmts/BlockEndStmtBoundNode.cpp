#include "BoundNodes/Stmts/BlockEndStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    BlockEndStmtBoundNode::BlockEndStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope }
    {
    }

    auto BlockEndStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BlockEndStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockEndStmtBoundNode::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto BlockEndStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto BlockEndStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const BlockEndStmtBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag{} };
    }

    auto BlockEndStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto BlockEndStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const BlockEndStmtBoundNode>
    {
        return shared_from_this();
    }

    auto BlockEndStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto BlockEndStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
    }

    auto BlockEndStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return {};
    }
}
