#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Attribute.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Type
{
    class IBase : public virtual BoundNode::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetOrCreateTypeCheckedType(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>>> = 0;
        virtual auto GetOrCreateLoweredType(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>> = 0;

        virtual auto GetSymbol() const -> Symbol::Type::IBase* = 0;
    };
}
