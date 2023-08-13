#pragma once

#include <vector>

#include "Nodes/Node.hpp"
#include "Ident.hpp"

namespace Ace
{
    class ITemplateNode :
        public virtual INode,
        public virtual ISymbolCreatableNode
    {
    public:
        virtual ~ITemplateNode() = default;

        virtual auto GetAST() const -> std::shared_ptr<const ITemplatableNode> = 0;

        virtual auto CollectImplParamNames() const -> std::vector<Ident> = 0;
        virtual auto CollectParamNames()     const -> std::vector<Ident> = 0;
    };
}
