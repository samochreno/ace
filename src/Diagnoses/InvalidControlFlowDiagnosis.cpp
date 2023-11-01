#include "Diagnoses/InvalidControlFlowDiagnosis.hpp"

#include <vector>

#include "ControlFlow.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/DiagnosisDiagnostics.hpp"
#include "Symbols/LabelSymbol.hpp"

namespace Ace
{
    static auto IsEnd(
        const std::vector<ControlFlowNode>::const_iterator nodeIt,
        const std::vector<std::vector<ControlFlowNode>::const_iterator>& ends
    ) -> bool
    {
        const auto matchingEndIt = std::find_if(begin(ends), end(ends),
        [&](const std::vector<ControlFlowNode>::const_iterator end)
        {
            return nodeIt == end;
        });

        return matchingEndIt != end(ends);
    }

    static auto FindLabelNode(
        const ControlFlowGraph& graph,
        LabelSymbol* const labelSymbol
    ) -> std::vector<ControlFlowNode>::const_iterator
    {
        return std::find_if(begin(graph.Nodes), end(graph.Nodes),
        [&](const ControlFlowNode& node)
        {
            return 
                (node.Kind == ControlFlowKind::Label) &&
                (node.LabelSymbol == labelSymbol);
        });
    }

    static auto IsEndReachableWithoutRet(
        const ControlFlowGraph& graph,
        const std::vector<ControlFlowNode>::const_iterator begin,
        const std::vector<std::vector<ControlFlowNode>::const_iterator>& ends
    ) -> bool
    {
        for (auto nodeIt = begin; nodeIt != end(graph.Nodes); ++nodeIt)
        {
            if (IsEnd(nodeIt, ends))
            {
                return false;
            }

            const auto& node = *nodeIt;

            switch (node.Kind)
            {
                case ControlFlowKind::Label:
                {
                    continue;
                }

                case ControlFlowKind::Jump:
                {
                    const auto labelNodeIt =
                        FindLabelNode(graph, node.LabelSymbol);
                    ACE_ASSERT(labelNodeIt != end(graph.Nodes));

                    auto newEnds = ends;
                    newEnds.push_back(nodeIt);

                    return IsEndReachableWithoutRet(
                        graph,
                        labelNodeIt,
                        newEnds
                    );
                }

                case ControlFlowKind::ConditionalJump:
                {
                    const auto labelNodeIt =
                        FindLabelNode(graph, node.LabelSymbol);
                    ACE_ASSERT(labelNodeIt != end(graph.Nodes));

                    auto whenTrueEnds = ends;
                    whenTrueEnds.push_back(nodeIt);

                    const bool whenTrue = IsEndReachableWithoutRet(
                        graph,
                        labelNodeIt,
                        whenTrueEnds
                    );

                    const bool whenFalse = IsEndReachableWithoutRet(
                        graph,
                        nodeIt + 1,
                        ends
                    );

                    return whenTrue || whenFalse;
                }

                case ControlFlowKind::Ret:
                case ControlFlowKind::Exit:
                {
                    return false;
                }
            }
        }

        return true;
    }

    auto DiagnoseInvalidControlFlow(
        const SrcLocation& srcLocation,
        const ControlFlowGraph& graph
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsEndReachableWithoutRet(graph, begin(graph.Nodes), {}))
        {
            diagnostics.Add(CreateNotAllControlPathsRetError(srcLocation));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
