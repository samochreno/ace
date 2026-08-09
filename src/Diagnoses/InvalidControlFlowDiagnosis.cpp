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
        const std::vector<ControlFlowInstruction>::const_iterator instructionIt,
        const std::vector<std::vector<ControlFlowInstruction>::const_iterator>& ends
    ) -> bool
    {
        const auto matchingEndIt = std::find_if(
            begin(ends),
            end(ends),
            [&](const std::vector<ControlFlowInstruction>::const_iterator end)
            {
                return instructionIt == end;
            }
        );

        return matchingEndIt != end(ends);
    }

    static auto FindLabelInstruction(const ControlFlowGraph& graph, LabelSymbol* const labelSymbol)
        -> std::vector<ControlFlowInstruction>::const_iterator
    {
        return std::find_if(
            begin(graph.Instructions),
            end(graph.Instructions),
            [&](const ControlFlowInstruction& instruction)
            {
                return (instruction.Kind == ControlFlowKind::Label) &&
                       (instruction.LabelSymbol == labelSymbol);
            }
        );
    }

    static auto IsEndReachableWithoutRet(
        const ControlFlowGraph& graph,
        const std::vector<ControlFlowInstruction>::const_iterator begin,
        const std::vector<std::vector<ControlFlowInstruction>::const_iterator>& ends
    ) -> bool
    {
        for (auto instructionIt = begin; instructionIt != end(graph.Instructions); ++instructionIt)
        {
            if (IsEnd(instructionIt, ends))
            {
                return false;
            }

            const auto& instruction = *instructionIt;

            switch (instruction.Kind)
            {
                case ControlFlowKind::Label:
                {
                    continue;
                }

                case ControlFlowKind::Jump:
                {
                    const auto labelInstructionIt =
                        FindLabelInstruction(graph, instruction.LabelSymbol);
                    ACE_ASSERT(labelInstructionIt != end(graph.Instructions));

                    auto newEnds = ends;
                    newEnds.push_back(instructionIt);

                    return IsEndReachableWithoutRet(graph, labelInstructionIt, newEnds);
                }

                case ControlFlowKind::ConditionalJump:
                {
                    const auto labelInstructionIt =
                        FindLabelInstruction(graph, instruction.LabelSymbol);
                    ACE_ASSERT(labelInstructionIt != end(graph.Instructions));

                    auto whenTrueEnds = ends;
                    whenTrueEnds.push_back(instructionIt);

                    const bool whenTrue =
                        IsEndReachableWithoutRet(graph, labelInstructionIt, whenTrueEnds);

                    const bool whenFalse = IsEndReachableWithoutRet(graph, instructionIt + 1, ends);

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

    auto DiagnoseInvalidControlFlow(const SrcLocation& srcLocation, const ControlFlowGraph& graph)
        -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsEndReachableWithoutRet(graph, begin(graph.Instructions), {}))
        {
            diagnostics.Add(CreateNotAllControlPathsRetError(srcLocation));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
