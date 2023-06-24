#include "Symbol/Templatable.hpp"

#include "Symbol/Type/TemplateParam/Impl.hpp"
#include "Symbol/Type/TemplateParam/Normal.hpp"

namespace Ace::Symbol
{
    static auto IsParam(Symbol::Type::IBase* t_arg) -> bool
    {
        t_arg = t_arg->GetUnaliased();

        if (dynamic_cast<Symbol::Type::TemplateParam::Impl*>(t_arg))
            return true;

        if (dynamic_cast<Symbol::Type::TemplateParam::Normal*>(t_arg))
            return true;

        return false;
    }

    auto ITemplatable::IsTemplatePlaceholder() const -> bool
    {
        const auto implTemplateArgs = CollectImplTemplateArgs();
        const auto     templateArgs = CollectTemplateArgs();

        const auto implFoundIt = std::find_if(
            begin(implTemplateArgs),
            end  (implTemplateArgs),
            IsParam
        );
        if (implFoundIt != end(implTemplateArgs))
            return true;

        const auto foundIt = std::find_if(
            begin(templateArgs),
            end  (templateArgs),
            IsParam
        );
        if (foundIt != end(templateArgs))
            return true;

        return false;
    }
}
