#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    LabelStmtBoundNode::LabelStmtBoundNode(
        const SrcLocation& srcLocation,
        LabelSymbol* const symbol
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol }
    {
    }

    auto LabelStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LabelStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto LabelStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto LabelStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LabelStmtBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag{} };
    }

    auto LabelStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto LabelStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const LabelStmtBoundNode>
    {
        return shared_from_this();
    }

    auto LabelStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto LabelStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
    }

    auto LabelStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return std::vector{ CFANode{ CFANodeKind::Label, m_Symbol } };
    }

    auto LabelStmtBoundNode::GetSymbol() const -> LabelSymbol*
    {
        return m_Symbol;
    }
}
