#include "Symbol/Base.hpp"

#include <vector>

#include "Symbol/Variable/Parameter/Base.hpp"

namespace Ace::Symbol
{
    class IParameterized : public virtual Symbol::IBase
    {
    public:
        virtual ~IParameterized() = default;

        virtual auto CollectParameters() const -> std::vector<Symbol::Variable::Parameter::IBase*> = 0;
    };
}

