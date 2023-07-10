#pragma once

#include <vector>

#include "Nodes/Node.hpp"
#include "Identifier.hpp"

namespace Ace
{
    class ITemplateNode :
        public virtual INode,
        public virtual ISymbolCreatableNode
    {
    public:
        virtual ~ITemplateNode() = default;

        virtual auto CollectImplParamNames() const -> std::vector<Identifier> = 0;
        virtual auto CollectParamNames()     const -> std::vector<Identifier> = 0;
    };
}
