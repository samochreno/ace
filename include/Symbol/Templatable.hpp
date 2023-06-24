#pragma once

#include <vector>

#include "Symbol/Base.hpp"

namespace Ace::Symbol::Type
{
    class IBase;
}

namespace Ace::Symbol
{
    class ITemplatable : public virtual Symbol::IBase
    {
    public:
        virtual ~ITemplatable() = default;

        virtual auto CollectTemplateArgs()     const -> std::vector<Symbol::Type::IBase*> = 0;
        virtual auto CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*> = 0;

        virtual auto IsTemplatePlaceholder() const -> bool final;
    };
}
