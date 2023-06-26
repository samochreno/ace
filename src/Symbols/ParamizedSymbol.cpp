#include "Symbols/ParamizedSymbol.hpp"

namespace Ace
{
    auto IParamizedSymbol::CollectParamTypes() const -> std::vector<ITypeSymbol*>
    {
        const auto params = CollectParams();

        std::vector<ITypeSymbol*> types{};
        std::transform(
            begin(params),
            end  (params),
            back_inserter(types),
            [](IParamVarSymbol* const t_param)
            {
                return t_param->GetType();
            }
        );
        
        return types;
    }
}
