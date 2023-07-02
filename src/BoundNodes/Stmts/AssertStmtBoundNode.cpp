#include "BoundNodes/Stmts/AssertStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    AssertStmtBoundNode::AssertStmtBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_condition
    ) : m_Condition{ t_condition }
    {
    }

    auto AssertStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto AssertStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto AssertStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const AssertStmtBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (!mchConvertedAndCheckedCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const AssertStmtBoundNode>(
            mchConvertedAndCheckedCondition.Value
        );
        return CreateChanged(returnValue);
    }

    auto AssertStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto AssertStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto mchLoweredCondition =
            m_Condition->GetOrCreateLoweredExpr({});

        const auto condition = std::make_shared<const LogicalNegationExprBoundNode>(
            mchLoweredCondition.Value
        );

        const auto bodyScope = GetScope()->GetOrCreateChild({});

        const auto exitStmt = std::make_shared<const ExitStmtBoundNode>(
            bodyScope
        );

        const auto bodyStmt = std::make_shared<const BlockStmtBoundNode>(
            bodyScope,
            std::vector<std::shared_ptr<const IStmtBoundNode>>{ exitStmt }
        );

        const auto returnValue = std::make_shared<const IfStmtBoundNode>(
            GetScope(),
            std::vector<std::shared_ptr<const IExprBoundNode>>{ condition },
            std::vector<std::shared_ptr<const BlockStmtBoundNode>>{ bodyStmt }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    };

    auto AssertStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto AssertStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}