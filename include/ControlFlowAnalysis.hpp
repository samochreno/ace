#pragma once

#include "BoundNode/Stmt/Block.hpp"
#include "Symbol/Label.hpp"
#include "Diagnostics.hpp"

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
        Symbol::Label* LabelSymbol{};
    };

    class ControlFlowAnalysis
    {
    public:
        ControlFlowAnalysis(
            const std::shared_ptr<const BoundNode::Stmt::Block>& t_blockStmtNode
        );
        ~ControlFlowAnalysis() = default;

        auto IsEndReachableWithoutReturn() const -> bool;

    private:
        auto FindLabelStmt(
            const Symbol::Label* const t_labelSymbol
        ) const -> std::vector<ControlFlowStmt>::const_iterator;

        auto IsEndReachableWithoutReturn(
            const std::vector<ControlFlowStmt>::const_iterator& t_begin,
            const std::vector<std::vector<ControlFlowStmt>::const_iterator>& t_ends
        ) const -> bool;

        std::vector<ControlFlowStmt> m_Stmts{};
    };
}
