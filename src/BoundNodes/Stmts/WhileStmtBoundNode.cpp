#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    WhileStmtBoundNode::WhileStmtBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprBoundNode>& t_condition,
        const std::shared_ptr<const BlockStmtBoundNode>& t_body
    ) : m_Scope{ t_scope },
        m_Condition{ t_condition },
        m_Body{ t_body }
    {
    }

    auto WhileStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto WhileStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto WhileStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const WhileStmtBoundNode>>>
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

        ACE_TRY(mchCheckedBody, m_Body->GetOrCreateTypeChecked({
            t_context.ParentFunctionTypeSymbol
        }));

        if (
            !mchConvertedAndCheckedCondition.IsChanged &&
            !mchCheckedBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const WhileStmtBoundNode>(
            m_Scope,
            mchConvertedAndCheckedCondition.Value,
            mchCheckedBody.Value
        );
        return CreateChanged(returnValue);
    }

    auto WhileStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto WhileStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        // From:
        // while condition {
        //     body;
        // }
        // 
        // To:
        // goto continue;
        // start:
        // body;
        // continue:
        // gotoif condition start;

        auto startLabelSymbolOwned = std::make_unique<LabelSymbol>(
            m_Scope,
            SpecialIdentifier::CreateAnonymous()
        );

        auto* const startLabelSymbol = dynamic_cast<LabelSymbol*>(
            m_Scope->DefineSymbol(std::move(startLabelSymbolOwned)).Unwrap()
        );

        auto continueLabelSymbolOwned = std::make_unique<LabelSymbol>(
            m_Scope,
            SpecialIdentifier::CreateAnonymous()
        );

        auto* const continueLabelSymbol = dynamic_cast<LabelSymbol*>(
            m_Scope->DefineSymbol(std::move(continueLabelSymbolOwned)).Unwrap()
        );

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        stmts.push_back(std::make_shared<const NormalJumpStmtBoundNode>(
            m_Scope,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            startLabelSymbol
        ));

        stmts.push_back(m_Body);

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const ConditionalJumpStmtBoundNode>(
            m_Condition,
            startLabelSymbol
        ));

        const auto returnValue = std::make_shared<const GroupStmtBoundNode>(
            m_Scope,
            stmts
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto WhileStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto WhileStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
