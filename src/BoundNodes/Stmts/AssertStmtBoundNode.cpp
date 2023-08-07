#include "BoundNodes/Stmts/AssertStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "CFA.hpp"

namespace Ace
{
    AssertStmtBoundNode::AssertStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& condition
    ) : m_SrcLocation{ srcLocation },
        m_Condition{ condition }
    {
    }

    auto AssertStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AssertStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto AssertStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto AssertStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const AssertStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedCondition = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (checkedCondition == m_Condition)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const AssertStmtBoundNode>(
                GetSrcLocation(),
                checkedCondition
            ),
            diagnostics,
        };
    }

    auto AssertStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto AssertStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtBoundNode>
    {
        const auto loweredCondition = m_Condition->CreateLoweredExpr({});

        const auto condition = std::make_shared<const LogicalNegationExprBoundNode>(
            loweredCondition->GetSrcLocation(),
            loweredCondition
        );

        const auto bodyScope = GetScope()->GetOrCreateChild({});

        const auto exitStmt = std::make_shared<const ExitStmtBoundNode>(
            GetSrcLocation(),
            bodyScope
        );

        const auto bodyStmt = std::make_shared<const BlockStmtBoundNode>(
            GetSrcLocation(),
            bodyScope,
            std::vector<std::shared_ptr<const IStmtBoundNode>>{ exitStmt }
        );

        return std::make_shared<const IfStmtBoundNode>(
            GetSrcLocation(),
            GetScope(),
            std::vector<std::shared_ptr<const IExprBoundNode>>{ condition },
            std::vector<std::shared_ptr<const BlockStmtBoundNode>>{ bodyStmt }
        )->CreateLowered({});
    };

    auto AssertStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto AssertStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto AssertStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        ACE_UNREACHABLE();
    }
}
