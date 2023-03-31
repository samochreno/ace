#pragma once

#include "Symbol/Type/Alias/Base.hpp"

namespace Ace::Symbol::Type::Alias::TemplateArgument
{
    class IBase : public virtual Symbol::Type::Alias::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetIndex() const -> size_t = 0;
    };
}
