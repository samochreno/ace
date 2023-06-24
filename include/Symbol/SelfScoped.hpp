#pragma once

#include <memory>

#include "Symbol/Base.hpp"

namespace Ace::Symbol
{
    class ISelfScoped : public virtual Symbol::IBase
    {
    public:
        virtual ~ISelfScoped() = default;

        virtual auto GetSelfScope() const -> std::shared_ptr<Scope> = 0;
    };
}
