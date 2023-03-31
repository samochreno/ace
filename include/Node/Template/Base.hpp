#pragma once

#include <vector>
#include <string>

#include "Node/Base.hpp"

namespace Ace::Node::Template
{
    class IBase :
        public virtual Node::IBase,
        public virtual Node::ISymbolCreatable
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetImplParameters() const -> const std::vector<std::string>& = 0;
        virtual auto GetParameters() const -> const std::vector<std::string>& = 0;
    };
}
