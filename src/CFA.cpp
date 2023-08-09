#include "CFA.hpp"

#include "Symbols/LabelSymbol.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/CFADiagnostics.hpp"
#include "Assert.hpp"

namespace Ace
{
    static auto IsEnd(
        const std::vector<CFANode>::const_iterator nodeIt,
        const std::vector<std::vector<CFANode>::const_iterator>& ends
    ) -> bool
    {
        const auto matchingEndIt = std::find_if(begin(ends), end(ends),
        [&](const std::vector<CFANode>::const_iterator end)
        {
            return nodeIt == end;
        });

        return matchingEndIt != end(ends);
    }

    static auto FindLabelNode(
        const CFAGraph& graph,
        const LabelSymbol* const labelSymbol
    ) -> std::vector<CFANode>::const_iterator
    {
        return std::find_if(begin(graph.Nodes), end(graph.Nodes),
        [&](const CFANode& node)
        {
            return 
                (node.Kind == CFANodeKind::Label) &&
                (node.LabelSymbol == labelSymbol);
        });
    }

    static auto IsEndReachableWithoutReturn(
        const CFAGraph& graph,
        const std::vector<CFANode>::const_iterator begin,
        const std::vector<std::vector<CFANode>::const_iterator>& ends
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
                case CFANodeKind::Label:
                {
                    continue;
                }

                case CFANodeKind::Jump:
                {
                    const auto labelNodeIt = FindLabelNode(
                        graph,
                        node.LabelSymbol
                    );
                    ACE_ASSERT(labelNodeIt != end(graph.Nodes));

                    auto newEnds = ends;
                    newEnds.push_back(nodeIt);

                    return IsEndReachableWithoutReturn(
                        graph,
                        labelNodeIt,
                        newEnds
                    );
                }

                case CFANodeKind::ConditionalJump:
                {
                    const auto labelNodeIt = FindLabelNode(
                        graph,
                        node.LabelSymbol
                    );
                    ACE_ASSERT(labelNodeIt != end(graph.Nodes));

                    auto whenTrueEnds = ends;
                    whenTrueEnds.push_back(nodeIt);

                    const bool whenTrue = IsEndReachableWithoutReturn(
                        graph,
                        labelNodeIt,
                        whenTrueEnds
                    );

                    const bool whenFalse = IsEndReachableWithoutReturn(
                        graph,
                        nodeIt + 1,
                        ends
                    );

                    return whenTrue || whenFalse;
                }

                case CFANodeKind::Return:
                case CFANodeKind::Exit:
                {
                    return false;
                }
            }
        }

        return true;
    }

    auto CFA(
        const SrcLocation& srcLocation,
        const CFAGraph& graph
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsEndReachableWithoutReturn(graph, begin(graph.Nodes), {}))
        {
            diagnostics.Add(CreateNotAllControlPathsReturnError(
                srcLocation
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
