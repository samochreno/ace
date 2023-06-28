#pragma once

#include <vector>
#include <string>

#include "Nodes/Node.hpp"

namespace Ace
{
    class ITemplateNode :
        public virtual INode,
        public virtual ISymbolCreatableNode
    {
    public:
        virtual ~ITemplateNode() = default;

        virtual auto CollectImplParamNames() const -> std::vector<std::string> = 0;
        virtual auto CollectParamNames()     const -> std::vector<std::string> = 0;
    };
}
