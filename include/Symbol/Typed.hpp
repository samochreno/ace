#pragma once

#include "Symbol/Base.hpp"
#include "Symbol/Type/Base.hpp"

namespace Ace::Symbol
{
    class ITyped : public virtual Symbol::IBase
    {
    public:
        virtual ~ITyped() = default;

        virtual auto GetType() const -> Symbol::Type::IBase* = 0;
    };
}
