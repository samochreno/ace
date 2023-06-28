#pragma once

#include <string>

#include "Nodes/Node.hpp"

namespace Ace
{
    class ITypedNode :
        public virtual INode,
        public virtual ISymbolCreatableNode
    {
    public:
        virtual ~ITypedNode() = default;

        virtual auto GetName() const -> const std::string& = 0;
    };
}
