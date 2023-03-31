#pragma once

#include "Symbol/Variable/Base.hpp"

namespace Ace::Symbol::Variable::Parameter
{
    class IBase : public virtual Symbol::Variable::IBase
    {
    public:
        virtual ~IBase() = default;
    };
}
