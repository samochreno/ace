#pragma once

#include <memory>

#include "Nodes/Node.hpp"
#include "Nodes/TemplatableNode.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ITypeNode :
        public virtual INode,
        public virtual ISymbolCreatableNode,
        public virtual ITemplatableNode
    {
    public:
        virtual ~ITypeNode() = default;

        virtual auto CloneInScopeType(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const ITypeNode> = 0;

        virtual auto CreateBoundType() const -> Diagnosed<std::shared_ptr<const ITypeBoundNode>> = 0;
    };
}
