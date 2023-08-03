#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "Diagnostic.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    class ITypeBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypeBoundNode() = default;

        virtual auto CloneWithDiagnosticsType(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const ITypeBoundNode> = 0;
        virtual auto GetOrCreateTypeCheckedType(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const ITypeBoundNode>>> = 0;
        virtual auto GetOrCreateLoweredType(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const ITypeBoundNode>> = 0;

        virtual auto GetSymbol() const -> ITypeSymbol* = 0;
    };
}
