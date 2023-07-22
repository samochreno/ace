#pragma once

#include <memory>

#include "Nodes/Node.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class ITypeNode :
        public virtual INode,
        public virtual ISymbolCreatableNode
    {
    public:
        virtual ~ITypeNode() = default;

        virtual auto GetSelfScope() const -> std::shared_ptr<Scope> = 0;

        virtual auto CloneInScopeType(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const ITypeNode> = 0;

        virtual auto CreateBoundType() const -> Expected<std::shared_ptr<const ITypeBoundNode>> = 0;

        virtual auto GetName() const -> const Ident& = 0;
        virtual auto GetAccessModifier() const -> AccessModifier = 0;
    };
}
