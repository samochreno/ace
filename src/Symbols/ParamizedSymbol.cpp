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
            [](IParamVarSymbol* const param)
            {
                return param->GetType();
            }
        );
        
        return types;
    }
}
