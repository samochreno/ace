#pragma once

#include "Symbol/Var/Base.hpp"

namespace Ace::Symbol::Var::Param
{
    class IBase : public virtual Symbol::Var::IBase
    {
    public:
        virtual ~IBase() = default;
    };
}
