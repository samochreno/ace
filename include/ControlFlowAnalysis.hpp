#pragma once

#include "BoundNode/Statement/Block.hpp"
#include "Symbol/Label.hpp"
#include "Error.hpp"

namespace Ace
{
    enum class ControlFlowStatementKind
    {
        Label,
        NormalJump,
        ConditionalJump,
        Return,
        Exit,
    };

    struct ControlFlowStatement
    {
        ControlFlowStatementKind Kind{};
        Symbol::Label* LabelSymbol{};
    };

    class ControlFlowAnalysis
    {
    public:
        ControlFlowAnalysis(
            const std::shared_ptr<const BoundNode::Statement::Block>& t_blockStatementNode
        );
        ~ControlFlowAnalysis() = default;

        auto IsEndReachableWithoutReturn() const -> bool;

    private:
        auto FindLabelStatement(
            const Symbol::Label* const t_labelSymbol
        ) const -> std::vector<ControlFlowStatement>::const_iterator;

        auto IsEndReachableWithoutReturn(
            const std::vector<ControlFlowStatement>::const_iterator& t_begin,
            const std::vector<std::vector<ControlFlowStatement>::const_iterator>& t_ends
        ) const -> bool;

        std::vector<ControlFlowStatement> m_Statements{};
    };
}
