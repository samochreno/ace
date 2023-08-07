#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdent.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "CFA.hpp"

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

    auto IfStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Conditions);
        AddChildren(children, m_Bodies);

        return children;
    }

    auto IfStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IfStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };

        std::vector<std::shared_ptr<const IExprBoundNode>> checkedConditions{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(checkedConditions),
            [&](const std::shared_ptr<const IExprBoundNode>& condition)
            {
                return diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
                    condition,
                    typeInfo
                ));
            }
        );

        std::vector<std::shared_ptr<const BlockStmtBoundNode>> checkedBodies{};
        std::transform(
            begin(m_Bodies),
            end  (m_Bodies),
            back_inserter(checkedBodies),
            [&](const std::shared_ptr<const BlockStmtBoundNode>& body)
            {
                return diagnostics.Collect(body->CreateTypeChecked({
                    context.ParentFunctionTypeSymbol
                }));
            }
        );

        if (
            (checkedConditions == m_Conditions) &&
            (checkedBodies == m_Bodies)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const IfStmtBoundNode>(
                GetSrcLocation(),
                GetScope(),
                checkedConditions,
                checkedBodies
            ),
            diagnostics,
        };
    }

    auto IfStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto IfStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtBoundNode>
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

        return std::make_shared<const GroupStmtBoundNode>(
            GetSrcLocation(),
            m_Scope,
            stmts
        )->CreateLowered({});
    }

    auto IfStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto IfStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto IfStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        ACE_UNREACHABLE();
    }
}
