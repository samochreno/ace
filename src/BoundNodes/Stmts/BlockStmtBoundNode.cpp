#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockEndStmtBoundNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace
{
    BlockStmtBoundNode::BlockStmtBoundNode(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& t_stmts
    ) : m_SelfScope{ t_selfScope },
        m_Stmts{ t_stmts }
    {
    }
    
    auto BlockStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto BlockStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BlockStmtBoundNode>>>
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

        const auto returnValue = std::make_shared<const BlockStmtBoundNode>(
            m_SelfScope,
            mchCheckedContent.Value
        );
        return CreateChanged(returnValue);
    }

    auto BlockStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto BlockStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BlockStmtBoundNode>>
    {
        const auto mchLoweredStmts = TransformMaybeChangedVector(m_Stmts,
        [](const std::shared_ptr<const IStmtBoundNode>& t_stmt)
        {
            return t_stmt->GetOrCreateLoweredStmt({});
        });

        if (!mchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BlockStmtBoundNode>(
            m_SelfScope,
            mchLoweredStmts.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto BlockStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto BlockStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        const auto stmts = CreateExpanded();
        t_emitter.EmitFunctionBodyStmts(stmts);
    }

    auto BlockStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        auto stmts = m_Stmts;

        const auto blockEnd = std::make_shared<const BlockEndStmtBoundNode>(
            m_SelfScope
        );
        stmts.push_back(blockEnd);

        return stmts;
    }
}
