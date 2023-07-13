#pragma once

#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    enum class ControlFlowStmtKind
    {
        Label,
        NormalJump,
        ConditionalJump,
        Return,
        Exit,
    };

    struct ControlFlowStmt
    {
        ControlFlowStmtKind Kind{};
        LabelSymbol* LabelSymbol{};
    };

    class ControlFlowAnalysis
    {
    public:
        ControlFlowAnalysis(
            const std::shared_ptr<const BlockStmtBoundNode>& t_blockStmtNode
        );
        ~ControlFlowAnalysis() = default;

        auto IsEndReachableWithoutReturn() const -> bool;

    private:
        auto FindLabelStmt(
            const LabelSymbol* const t_labelSymbol
        ) const -> std::vector<ControlFlowStmt>::const_iterator;

        auto IsEndReachableWithoutReturn(
            const std::vector<ControlFlowStmt>::const_iterator t_begin,
            const std::vector<std::vector<ControlFlowStmt>::const_iterator>& t_ends
        ) const -> bool;

        std::vector<ControlFlowStmt> m_Stmts{};
    };
}
