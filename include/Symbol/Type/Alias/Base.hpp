#pragma once

#include <string>

#include "Symbol/Type/Base.hpp"

namespace Ace::Symbol::Type::Alias
{
    class IBase : public virtual Symbol::Type::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetAliasedType() const -> Symbol::Type::IBase* = 0;
    };
}
