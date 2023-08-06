#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    NormalJumpStmtBoundNode::NormalJumpStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        LabelSymbol* const labelSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LabelSymbol{ labelSymbol }
    {
    }

    auto NormalJumpStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalJumpStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalJumpStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto NormalJumpStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const NormalJumpStmtBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag{} };
    }

    auto NormalJumpStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto NormalJumpStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const NormalJumpStmtBoundNode>
    {
        return shared_from_this();
    }

    auto NormalJumpStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto NormalJumpStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        emitter.GetBlockBuilder().Builder.CreateBr(
            emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol)
        );
    }

    auto NormalJumpStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return std::vector{ CFANode{ CFANodeKind::Jump, m_LabelSymbol } };
    }
}
