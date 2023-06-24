#include "ControlFlowAnalysis.hpp"

#include "Asserts.hpp"
#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Stmt/Block.hpp"
#include "BoundNode/Stmt/Label.hpp"
#include "BoundNode/Stmt/Jump/Normal.hpp"
#include "BoundNode/Stmt/Jump/Conditional.hpp"
#include "BoundNode/Stmt/Return.hpp"
#include "BoundNode/Stmt/Exit.hpp"

namespace Ace
{
    static auto CreateStmt(
        const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmtNode
    ) -> Expected<ControlFlowStmt>
    {
        auto* const stmtNode = t_stmtNode.get();

        ControlFlowStmt self{};

        if (const auto* const labelStmt = dynamic_cast<const BoundNode::Stmt::Label*>(stmtNode))
        {
            self.Kind = ControlFlowStmtKind::Label;
            self.LabelSymbol = labelStmt->GetLabelSymbol();
        }
        else if (const auto* const normalJumpStmt = dynamic_cast<const BoundNode::Stmt::Jump::Normal*>(stmtNode))
        {
            self.Kind = ControlFlowStmtKind::NormalJump;
            self.LabelSymbol = normalJumpStmt->GetLabelSymbol();
        }
        else if (const auto* const conditionalJumpStmt = dynamic_cast<const BoundNode::Stmt::Jump::Conditional*>(stmtNode))
        {
            self.Kind = ControlFlowStmtKind::ConditionalJump;
            self.LabelSymbol = conditionalJumpStmt->GetLabelSymbol();
        }
        else if (const auto* const returnStmt = dynamic_cast<const BoundNode::Stmt::Return*>(stmtNode))
        {
            self.Kind = ControlFlowStmtKind::Return;
        }
        else if (const auto* const exitStmt = dynamic_cast<const BoundNode::Stmt::Exit*>(stmtNode))
        {
            self.Kind = ControlFlowStmtKind::Exit;
        }
        else
        {
            ACE_TRY_UNREACHABLE();
        }

        return self;
    }

    ControlFlowAnalysis::ControlFlowAnalysis(
        const std::shared_ptr<const BoundNode::Stmt::Block>& t_blockStmtNode
    )
    {
        const auto stmtNodes = t_blockStmtNode->CreateExpanded();

        std::for_each(begin(stmtNodes), end(stmtNodes),
        [&](const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmtNode)
        {
            const auto expStmt = CreateStmt(t_stmtNode);
            if (!expStmt)
                return;

            m_Stmts.push_back(expStmt.Unwrap());
        });
    }

    auto ControlFlowAnalysis::IsEndReachableWithoutReturn() const -> bool
    {
        return IsEndReachableWithoutReturn(begin(m_Stmts), {});
    }

    auto ControlFlowAnalysis::FindLabelStmt(
        const Symbol::Label* const t_labelSymbol
    ) const -> std::vector<ControlFlowStmt>::const_iterator
    {
        const auto foundIt = std::find_if(
            begin(m_Stmts),
            end  (m_Stmts),
            [&](const ControlFlowStmt& t_stmt)
            {
                return 
                    (t_stmt.Kind == ControlFlowStmtKind::Label) &&
                    (t_stmt.LabelSymbol == t_labelSymbol);
            }
        );

        ACE_ASSERT(foundIt != end(m_Stmts));
        return foundIt;
    }

    static auto IsEnd(
        const std::vector<std::vector<ControlFlowStmt>::const_iterator>& t_ends,
        const std::vector<ControlFlowStmt>::const_iterator& t_stmtIt
    ) -> bool
    {
        const auto foundEndIt = std::find_if(begin(t_ends), end(t_ends),
        [&](const std::vector<ControlFlowStmt>::const_iterator& t_end)
        {
            return t_stmtIt == t_end;
        });
        if (foundEndIt != end(t_ends))
            return true;

        return false;
    }

    auto ControlFlowAnalysis::IsEndReachableWithoutReturn(
        const std::vector<ControlFlowStmt>::const_iterator& t_begin,
        const std::vector<std::vector<ControlFlowStmt>::const_iterator>& t_ends
    ) const -> bool
    {
        for (
            auto stmtIt = t_begin;
            stmtIt != end(m_Stmts);
            ++stmtIt
            )
        {
            if (IsEnd(t_ends, stmtIt))
            {
                return false;
            }

            const auto& stmt = *stmtIt;

            switch (stmt.Kind)
            {
                case ControlFlowStmtKind::Label:
                {
                    continue;
                }

                case ControlFlowStmtKind::NormalJump:
                {
                    const auto labelStmtIt = FindLabelStmt(
                        stmt.LabelSymbol
                    );

                    auto ends = t_ends;
                    ends.push_back(stmtIt);

                    return IsEndReachableWithoutReturn(
                        labelStmtIt,
                        ends
                    );
                }

                case ControlFlowStmtKind::ConditionalJump:
                {
                    const auto labelStmtIt = FindLabelStmt(
                        stmt.LabelSymbol
                    );

                    auto whenTrueEnds = t_ends;
                    whenTrueEnds.push_back(stmtIt);

                    const bool whenTrue = IsEndReachableWithoutReturn(
                        labelStmtIt,
                        whenTrueEnds
                    );

                    const bool whenFalse = IsEndReachableWithoutReturn(
                        stmtIt + 1,
                        t_ends
                    );

                    return whenTrue || whenFalse;
                }

                case ControlFlowStmtKind::Return:
                case ControlFlowStmtKind::Exit:
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
