#include "BoundNodes/BoundNode.hpp"

#include "Compilation.hpp"

namespace Ace
{
    auto IBoundNode::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
    auto IBoundNode::CollectDiagnostics() const -> DiagnosticBag
    {
        DiagnosticBag diagnostics{};

        const auto children = CollectChildren();
        std::for_each(begin(children), end(children),
        [&](const IBoundNode* const child)
        {
            diagnostics.Add(child->GetDiagnostics());
        });

        diagnostics.Add(GetDiagnostics());

        return diagnostics;
    }
#pragma clang diagnostic pop
}
