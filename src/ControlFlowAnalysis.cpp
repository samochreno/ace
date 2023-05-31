#include "ControlFlowAnalysis.hpp"

#include "Asserts.hpp"
#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "BoundNode/Statement/Jump/Normal.hpp"
#include "BoundNode/Statement/Jump/Conditional.hpp"
#include "BoundNode/Statement/Return.hpp"
#include "BoundNode/Statement/Exit.hpp"

namespace Ace
{
    static auto CreateStatement(
        const std::shared_ptr<const BoundNode::Statement::IBase>& t_statementNode
    ) -> Expected<ControlFlowStatement>
    {
        auto* const statementNode = t_statementNode.get();

        ControlFlowStatement self{};

        if (const auto* const labelStatement = dynamic_cast<const BoundNode::Statement::Label*>(statementNode))
        {
            self.Kind = ControlFlowStatementKind::Label;
            self.LabelSymbol = labelStatement->GetLabelSymbol();
        }
        else if (const auto* const normalJumpStatement = dynamic_cast<const BoundNode::Statement::Jump::Normal*>(statementNode))
        {
            self.Kind = ControlFlowStatementKind::NormalJump;
            self.LabelSymbol = normalJumpStatement->GetLabelSymbol();
        }
        else if (const auto* const conditionalJumpStatement = dynamic_cast<const BoundNode::Statement::Jump::Conditional*>(statementNode))
        {
            self.Kind = ControlFlowStatementKind::ConditionalJump;
            self.LabelSymbol = conditionalJumpStatement->GetLabelSymbol();
        }
        else if (const auto* const returnStatement = dynamic_cast<const BoundNode::Statement::Return*>(statementNode))
        {
            self.Kind = ControlFlowStatementKind::Return;
        }
        else if (const auto* const exitStatement = dynamic_cast<const BoundNode::Statement::Exit*>(statementNode))
        {
            self.Kind = ControlFlowStatementKind::Exit;
        }
        else
        {
            ACE_TRY_UNREACHABLE();
        }

        return self;
    }

    ControlFlowAnalysis::ControlFlowAnalysis(
        const std::shared_ptr<const BoundNode::Statement::Block>& t_blockStatementNode
    )
    {
        const auto statementNodes = t_blockStatementNode->CreateExpanded();

        std::for_each(begin(statementNodes), end(statementNodes),
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statementNode)
        {
            const auto expStatement = CreateStatement(t_statementNode);
            if (!expStatement)
                return;

            m_Statements.push_back(expStatement.Unwrap());
        });
    }

    auto ControlFlowAnalysis::IsEndReachableWithoutReturn() const -> bool
    {
        return IsEndReachableWithoutReturn(begin(m_Statements), {});
    }

    auto ControlFlowAnalysis::FindLabelStatement(
        const Symbol::Label* const t_labelSymbol
    ) const -> std::vector<ControlFlowStatement>::const_iterator
    {
        const auto foundIt = std::find_if(
            begin(m_Statements),
            end  (m_Statements),
            [&](const ControlFlowStatement& t_statement)
            {
                return 
                    (t_statement.Kind == ControlFlowStatementKind::Label) &&
                    (t_statement.LabelSymbol == t_labelSymbol);
            }
        );

        ACE_ASSERT(foundIt != end(m_Statements));
        return foundIt;
    }

    static auto IsEnd(
        const std::vector<std::vector<ControlFlowStatement>::const_iterator>& t_ends,
        const std::vector<ControlFlowStatement>::const_iterator& t_statementIt
    ) -> bool
    {
        const auto foundEndIt = std::find_if(begin(t_ends), end(t_ends),
        [&](const std::vector<ControlFlowStatement>::const_iterator& t_end)
        {
            return t_statementIt == t_end;
        });
        if (foundEndIt != end(t_ends))
            return true;

        return false;
    }

    auto ControlFlowAnalysis::IsEndReachableWithoutReturn(
        const std::vector<ControlFlowStatement>::const_iterator& t_begin,
        const std::vector<std::vector<ControlFlowStatement>::const_iterator>& t_ends
    ) const -> bool
    {
        for (
            auto statementIt = t_begin;
            statementIt != end(m_Statements);
            ++statementIt
            )
        {
            if (IsEnd(t_ends, statementIt))
                return false;

            const auto& statement = *statementIt;

            switch (statement.Kind)
            {
                case ControlFlowStatementKind::Label:
                {
                    continue;
                }

                case ControlFlowStatementKind::NormalJump:
                {
                    const auto labelStatementIt = FindLabelStatement(
                        statement.LabelSymbol
                    );

                    auto ends = t_ends;
                    ends.push_back(statementIt);

                    return IsEndReachableWithoutReturn(
                        labelStatementIt,
                        ends
                    );
                }

                case ControlFlowStatementKind::ConditionalJump:
                {
                    const auto labelStatementIt = FindLabelStatement(
                        statement.LabelSymbol
                    );

                    auto whenTrueEnds = t_ends;
                    whenTrueEnds.push_back(statementIt);

                    const bool whenTrue = IsEndReachableWithoutReturn(
                        labelStatementIt,
                        whenTrueEnds
                    );

                    const bool whenFalse = IsEndReachableWithoutReturn(
                        statementIt + 1,
                        t_ends
                    );

                    return whenTrue || whenFalse;
                }

                case ControlFlowStatementKind::Return:
                case ControlFlowStatementKind::Exit:
                {
                    return false;
                }

                default:
                {
                    ACE_UNREACHABLE();
                }
            }
        }

        return true;
    }
}
