#include "Symbol/Type/TemplateParameter/Normal.hpp"

#include <vector>

namespace Ace::Symbol::Type::TemplateParameter
{
    auto Normal::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }

    auto Normal::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }
}

