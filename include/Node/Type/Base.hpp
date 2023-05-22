#pragma once

#include <memory>
#include <string>

#include "Node/Base.hpp"
#include "BoundNode/Type/Base.hpp"
#include "Error.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Node::Type
{
    class IBase :
        public virtual Node::IBase,
        public virtual Node::ISymbolCreatable
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetSelfScope() const -> std::shared_ptr<Scope> = 0;

        virtual auto CloneInScopeType(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Type::IBase> = 0;

        virtual auto CreateBoundType() const -> Expected<std::shared_ptr<const BoundNode::Type::IBase>> = 0;

        virtual auto GetName() const -> const std::string& = 0;
        virtual auto GetAccessModifier() const -> AccessModifier = 0;
    };
}
