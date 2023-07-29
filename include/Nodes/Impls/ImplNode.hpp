#pragma once

#include "Nodes/Node.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class IImplNode : public virtual INode
    {
    public:
        virtual ~IImplNode() = default;

        virtual auto DefineAssociations() const -> Expected<void> = 0;
    };
}
