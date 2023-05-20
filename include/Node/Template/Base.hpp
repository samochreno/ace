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

        virtual auto CollectImplParameterNames() const -> std::vector<std::string> = 0;
        virtual auto CollectParameterNames()     const -> std::vector<std::string> = 0;
    };
}
