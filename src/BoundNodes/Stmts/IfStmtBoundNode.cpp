#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdentifier.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    IfStmtBoundNode::IfStmtBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_conditions,
        const std::vector<std::shared_ptr<const BlockStmtBoundNode>>& t_bodies
    ) : m_Scope{ t_scope },
        m_Conditions{ t_conditions },
        m_Bodies{ t_bodies }
    {
    }

    auto IfStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto IfStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Conditions);
        AddChildren(children, m_Bodies);

        return children;
    }

    auto IfStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IfStmtBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedConditions, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Conditions,
            typeInfo
        ));

        ACE_TRY(mchCheckedBodies, TransformExpectedMaybeChangedVector(m_Bodies,
        [&](const std::shared_ptr<const BlockStmtBoundNode>& t_body)
        {
            return t_body->GetOrCreateTypeChecked({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (
            !mchConvertedAndCheckedConditions.IsChanged &&
            !mchCheckedBodies.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const IfStmtBoundNode>(
            m_Scope,
            mchConvertedAndCheckedConditions.Value,
            mchCheckedBodies.Value
        );
        return CreateChanged(returnValue);
    }

    auto IfStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto IfStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        // From:
        // if condition_0 {
        //     body_0;
        // } elif condition_1 {
        //     body_1;
        // } else {
        //     body_2;
        // }
        // 
        // To: 
        // gotoif !condition_0 label_0;
        // body_0;
        // goto label_2;
        // 
        // label_0:
        // gotoif !condition_1 label_1;
        // body_1;
        // goto label_2;
        // 
        // label_1:
        // body_2;
        // 
        // label_2:

        const bool hasElse = m_Bodies.size() > m_Conditions.size();

        std::vector<LabelSymbol*> labelSymbols{};
        const size_t labelCount = m_Conditions.size() + (hasElse ? 1 : 0);
        for (size_t i = 0; i < labelCount; i++)
        {
            auto labelSymbolOwned = std::make_unique<LabelSymbol>(
                m_Scope,
                SpecialIdentifier::CreateAnonymous()
            );

            auto* const labelSymbol = dynamic_cast<LabelSymbol*>(
                m_Scope->DefineSymbol(std::move(labelSymbolOwned)).Unwrap()
            );

            labelSymbols.push_back(labelSymbol);
        }

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        for (size_t i = 0; i < m_Conditions.size(); i++)
        {
            const bool isFirstBody = i == 0;
            const bool isLastBody  = i == (m_Bodies.size() - 1);

            if (!isFirstBody)
            {
                stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
                    labelSymbols.at(i - 1)
                ));
            }

            const auto condition = std::make_shared<const LogicalNegationExprBoundNode>(
                m_Conditions.at(i)
            );

            stmts.push_back(std::make_shared<const ConditionalJumpStmtBoundNode>(
                condition,
                labelSymbols.at(i)
            ));

            stmts.push_back(m_Bodies.at(i));

            if (!isLastBody)
            {
                stmts.push_back(std::make_shared<const NormalJumpStmtBoundNode>(
                    m_Scope,
                    labelSymbols.back()
                ));
            }
        }

        if (hasElse)
        {
            stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
                labelSymbols.rbegin()[1]
            ));

            stmts.push_back(m_Bodies.back());
        }

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            labelSymbols.back()
        ));

        const auto returnValue = std::make_shared<const GroupStmtBoundNode>(
            m_Scope,
            stmts
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto IfStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto IfStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
