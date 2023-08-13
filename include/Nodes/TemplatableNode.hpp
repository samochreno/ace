#pragma once

#include "Nodes/Node.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class ITemplatableNode : public virtual INode
    {
    public:
        virtual auto GetTemplateSelfScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetTemplateName() const -> const Ident& = 0;
        virtual auto GetTemplateAccessModifier() const -> AccessModifier = 0;
    };
}
