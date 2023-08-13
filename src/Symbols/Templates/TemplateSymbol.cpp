#include "Symbols/Templates/TemplateSymbol.hpp"

#include <vector>

#include "Nodes/TemplatableNode.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

namespace Ace
{
    auto ITemplateSymbol::GetAccessModifier() const -> AccessModifier
    {
        return GetAST()->GetTemplateAccessModifier();
    }

    auto ITemplateSymbol::CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*>
    {
        return GetAST()->GetTemplateSelfScope()->CollectSymbols<ImplTemplateParamTypeSymbol>();
    }

    auto ITemplateSymbol::CollectParams() const -> std::vector<NormalTemplateParamTypeSymbol*>
    {
        return GetAST()->GetTemplateSelfScope()->CollectSymbols<NormalTemplateParamTypeSymbol>();
    }
}
