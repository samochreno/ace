#include "Symbol/Paramized.hpp"

namespace Ace::Symbol
{
    auto IParamized::CollectParamTypes() const -> std::vector<Symbol::Type::IBase*>
    {
        const auto params = CollectParams();

        std::vector<Symbol::Type::IBase*> types{};
        std::transform(
            begin(params),
            end  (params),
            back_inserter(types),
            [](Symbol::Var::Param::IBase* const t_param)
            {
                return t_param->GetType();
            }
        );
        
        return types;
    }
}
