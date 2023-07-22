#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdent.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    IfStmtBoundNode::IfStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& conditions,
        const std::vector<std::shared_ptr<const BlockStmtBoundNode>>& bodies
    ) : m_Scope{ scope },
        m_Conditions{ conditions },
        m_Bodies{ bodies }
    {
    }

    auto IfStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const StmtTypeCheckingContext& context
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
        [&](const std::shared_ptr<const BlockStmtBoundNode>& body)
        {
            return body->GetOrCreateTypeChecked({
                context.ParentFunctionTypeSymbol
            });
        }));

        if (
            !mchConvertedAndCheckedConditions.IsChanged &&
            !mchCheckedBodies.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const IfStmtBoundNode>(
            GetSrcLocation(),
            GetScope(),
            mchConvertedAndCheckedConditions.Value,
            mchCheckedBodies.Value
        ));
    }

    auto IfStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto IfStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
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

        const auto lastBody = m_Bodies.back();

        std::vector<LabelSymbol*> labelSymbols{};

        const size_t labelCount = hasElse ?
            (m_Conditions.size() + 1) :
            (m_Conditions.size());

        for (size_t i = 0; i < labelCount; i++)
        {
            const auto labelSrcLocation = [&]()
            {
                if (i == (labelCount - 1))
                {
                    return lastBody->GetSrcLocation().CreateFirst();
                }

                if (hasElse && (i == (labelCount - 2)))
                {
                    return lastBody->GetSrcLocation().CreateLast();
                }

                return m_Conditions.at(i + 1)->GetSrcLocation();
            }();
            const Ident labelName
            {
                labelSrcLocation,
                SpecialIdent::CreateAnonymous()
            };
            auto labelSymbolOwned = std::make_unique<LabelSymbol>(
                m_Scope,
                labelName
            );

            auto* const labelSymbol = dynamic_cast<LabelSymbol*>(
                m_Scope->DefineSymbol(std::move(labelSymbolOwned)).Unwrap()
            );

            labelSymbols.push_back(labelSymbol);
        }

        auto* const lastLabelSymbol = labelSymbols.back();

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        for (size_t i = 0; i < m_Conditions.size(); i++)
        {
            const bool isFirstBody = i == 0;
            const bool isLastBody  = i == (m_Bodies.size() - 1);

            if (!isFirstBody)
            {
                auto* const labelSymbol = labelSymbols.at(i - 1);
                stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
                    labelSymbol->GetName().SrcLocation,
                    labelSymbol
                ));
            }

            const auto condition = std::make_shared<const LogicalNegationExprBoundNode>(
                m_Conditions.at(i)->GetSrcLocation(),
                m_Conditions.at(i)
            );

            stmts.push_back(std::make_shared<const ConditionalJumpStmtBoundNode>(
                condition->GetSrcLocation(),
                condition,
                labelSymbols.at(i)
            ));

            const auto body = m_Bodies.at(i);
            stmts.push_back(body);

            if (!isLastBody)
            {
                stmts.push_back(std::make_shared<const NormalJumpStmtBoundNode>(
                    body->GetSrcLocation().CreateLast(),
                    GetScope(),
                    lastLabelSymbol
                ));
            }
        }

        if (hasElse)
        {
            auto* const elseLabelSymbol = labelSymbols.rbegin()[1];

            stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
                elseLabelSymbol->GetName().SrcLocation,
                elseLabelSymbol
            ));

            stmts.push_back(lastBody);
        }

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            lastLabelSymbol->GetName().SrcLocation,
            lastLabelSymbol
        ));

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            GetSrcLocation(),
            m_Scope,
            stmts
        )->GetOrCreateLowered(context).Value);
    }

    auto IfStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto IfStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
