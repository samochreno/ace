#include "ControlFlowAnalysis.hpp"

#include "Assert.hpp"
#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"

namespace Ace
{
    static auto CreateStmt(
        const std::shared_ptr<const IStmtBoundNode>& stmtNode
    ) -> Expected<ControlFlowStmt>
    {
        ControlFlowStmt self{};

        if (const auto* const labelStmt = dynamic_cast<const LabelStmtBoundNode*>(stmtNode.get()))
        {
            self.Kind = ControlFlowStmtKind::Label;
            self.LabelSymbol = labelStmt->GetSymbol();
        }
        else if (const auto* const normalJumpStmt = dynamic_cast<const NormalJumpStmtBoundNode*>(stmtNode.get()))
        {
            self.Kind = ControlFlowStmtKind::NormalJump;
            self.LabelSymbol = normalJumpStmt->GetLabelSymbol();
        }
        else if (const auto* const conditionalJumpStmt = dynamic_cast<const ConditionalJumpStmtBoundNode*>(stmtNode.get()))
        {
            self.Kind = ControlFlowStmtKind::ConditionalJump;
            self.LabelSymbol = conditionalJumpStmt->GetLabelSymbol();
        }
        else if (const auto* const returnStmt = dynamic_cast<const ReturnStmtBoundNode*>(stmtNode.get()))
        {
            self.Kind = ControlFlowStmtKind::Return;
        }
        else if (const auto* const exitStmt = dynamic_cast<const ExitStmtBoundNode*>(stmtNode.get()))
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
        const std::shared_ptr<const BlockStmtBoundNode>& blockStmtNode
    )
    {
        const auto stmtNodes = blockStmtNode->CreateExpanded();

        std::for_each(begin(stmtNodes), end(stmtNodes),
        [&](const std::shared_ptr<const IStmtBoundNode>& stmtNode)
        {
            const auto expStmt = CreateStmt(stmtNode);
            if (!expStmt)
            {
                return;
            }

            m_Stmts.push_back(expStmt.Unwrap());
        });
    }

    auto ControlFlowAnalysis::IsEndReachableWithoutReturn() const -> bool
    {
        return IsEndReachableWithoutReturn(begin(m_Stmts), {});
    }

    auto ControlFlowAnalysis::FindLabelStmt(
        const LabelSymbol* const labelSymbol
    ) const -> std::vector<ControlFlowStmt>::const_iterator
    {
        const auto matchingLabelStmtIt = std::find_if(
            begin(m_Stmts),
            end  (m_Stmts),
            [&](const ControlFlowStmt& stmt)
            {
                return 
                    (stmt.Kind == ControlFlowStmtKind::Label) &&
                    (stmt.LabelSymbol == labelSymbol);
            }
        );

        ACE_ASSERT(matchingLabelStmtIt != end(m_Stmts));
        return matchingLabelStmtIt;
    }

    static auto IsEnd(
        const std::vector<std::vector<ControlFlowStmt>::const_iterator>& ends,
        const std::vector<ControlFlowStmt>::const_iterator stmtIt
    ) -> bool
    {
        const auto matchingEndIt = std::find_if(begin(ends), end(ends),
        [&](const std::vector<ControlFlowStmt>::const_iterator end)
        {
            return stmtIt == end;
        });

        return matchingEndIt != end(ends);
    }

    auto ControlFlowAnalysis::IsEndReachableWithoutReturn(
        const std::vector<ControlFlowStmt>::const_iterator begin,
        const std::vector<std::vector<ControlFlowStmt>::const_iterator>& ends
    ) const -> bool
    {
        for (
            auto stmtIt = begin;
            stmtIt != end(m_Stmts);
            ++stmtIt
            )
        {
            if (IsEnd(ends, stmtIt))
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

                    auto newEnds = ends;
                    newEnds.push_back(stmtIt);

                    return IsEndReachableWithoutReturn(
                        labelStmtIt,
                        newEnds
                    );
                }

                case ControlFlowStmtKind::ConditionalJump:
                {
                    const auto labelStmtIt = FindLabelStmt(
                        stmt.LabelSymbol
                    );

                    auto whenTrueEnds = ends;
                    whenTrueEnds.push_back(stmtIt);

                    const bool whenTrue = IsEndReachableWithoutReturn(
                        labelStmtIt,
                        whenTrueEnds
                    );

                    const bool whenFalse = IsEndReachableWithoutReturn(
                        stmtIt + 1,
                        ends
                    );

                    return whenTrue || whenFalse;
                }

                case ControlFlowStmtKind::Return:
                case ControlFlowStmtKind::Exit:
                {
                    return false;
                }
            }
        }

        return true;
    }
}
