#pragma once

#include "Symbol/Base.hpp"

namespace Ace
{
    class Scope;
}

namespace Ace::Symbol
{
    class ISelfScoped : public virtual Symbol::IBase
    {
    public:
        virtual ~ISelfScoped() = default;

        virtual auto GetSelfScope() const -> Scope* = 0;
    };
}
