#pragma once

#include "Nodes/Node.hpp"
#include "Ident.hpp"

namespace Ace
{
    class ITypedNode :
        public virtual INode,
        public virtual ISymbolCreatableNode
    {
    public:
        virtual ~ITypedNode() = default;

        virtual auto GetName() const -> const Ident& = 0;
    };
}
