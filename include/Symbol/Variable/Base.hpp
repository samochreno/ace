#pragma once

#include "Symbol/Base.hpp"
#include "Symbol/Typed.hpp"

namespace Ace::Symbol::Variable
{
    class IBase : public virtual Symbol::IBase, public virtual Symbol::ITyped
    {
    public:
        virtual ~IBase() = default;
    };
}
