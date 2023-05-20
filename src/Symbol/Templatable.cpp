#include "Symbol/Templatable.hpp"

#include "Symbol/Type/TemplateParameter/Impl.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"

namespace Ace::Symbol
{
    static auto IsParameter(Symbol::Type::IBase* t_argument) -> bool
    {
        t_argument = t_argument->GetUnaliased();

        if (dynamic_cast<Symbol::Type::TemplateParameter::Impl*>(t_argument))
            return true;

        if (dynamic_cast<Symbol::Type::TemplateParameter::Normal*>(t_argument))
            return true;

        return false;
    }

    auto ITemplatable::IsTemplatePlaceholder() const -> bool
    {
        const auto implTemplateArguments = CollectImplTemplateArguments();
        const auto     templateArguments = CollectTemplateArguments();

        const auto implFoundIt = std::find_if(
            begin(implTemplateArguments),
            end  (implTemplateArguments),
            IsParameter
        );
        if (implFoundIt != end(implTemplateArguments))
            return true;

        const auto foundIt = std::find_if(
            begin(templateArguments),
            end  (templateArguments),
            IsParameter
        );
        if (foundIt != end(templateArguments))
            return true;

        return false;
    }
}

