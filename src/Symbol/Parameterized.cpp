#include "Symbol/Parameterized.hpp"

namespace Ace::Symbol
{
    auto IParameterized::CollectParameterTypes() const -> std::vector<Symbol::Type::IBase*>
    {
        const auto parameters = CollectParameters();

        std::vector<Symbol::Type::IBase*> types{};
        std::transform(
            begin(parameters),
            end  (parameters),
            back_inserter(types),
            [](Symbol::Variable::Parameter::IBase* const t_parameter)
            {
                return t_parameter->GetType();
            }
        );
        
        return types;
    }
}

