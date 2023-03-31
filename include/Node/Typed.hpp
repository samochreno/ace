#pragma once

#include <string>

#include "Node/Base.hpp"

namespace Ace::Node
{
    class ITyped :
        public virtual Node::IBase,
        public virtual Node::ISymbolCreatable
    {
    public:
        virtual ~ITyped() = default;

        virtual auto GetName() const -> const std::string& = 0;
    };
}
