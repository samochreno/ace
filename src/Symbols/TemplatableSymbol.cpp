#include "Symbols/TemplatableSymbol.hpp"

#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

namespace Ace
{
    static auto IsParam(const ITypeSymbol* arg) -> bool
    {
        arg = arg->GetUnaliased();

        if (dynamic_cast<const ImplTemplateParamTypeSymbol*>(arg))
        {
            return true;
        }

        if (dynamic_cast<const NormalTemplateParamTypeSymbol*>(arg))
        {
            return true;
        }

        return false;
    }

    auto ITemplatableSymbol::IsTemplatePlaceholder() const -> bool
    {
        const auto* const typeSymbol = dynamic_cast<const ITypeSymbol*>(this);
        if (typeSymbol && IsParam(typeSymbol))
        {
            return true;
        }

        const auto implTemplateArgs = CollectImplTemplateArgs();
        const auto     templateArgs = CollectTemplateArgs();

        const auto implParameterIt = std::find_if(
            begin(implTemplateArgs),
            end  (implTemplateArgs),
            IsParam
        );
        if (implParameterIt != end(implTemplateArgs))
        {
            return true;
        }

        const auto parameterIt = std::find_if(
            begin(templateArgs),
            end  (templateArgs),
            IsParam
        );
        if (parameterIt != end(templateArgs))
        {
            return true;
        }

        return false;
    }
}
