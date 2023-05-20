#include "Symbol/Type/TemplateParameter/Impl.hpp"

#include <vector>

namespace Ace::Symbol::Type::TemplateParameter
{
    auto Impl::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }

    auto Impl::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }
}
