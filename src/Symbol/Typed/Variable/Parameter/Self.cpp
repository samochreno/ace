#include "Symbol/Variable/Parameter/Self.hpp"

#include <string>

namespace Ace::Symbol::Variable::Parameter
{
    auto Self::GetName() const -> const std::string&
    {
        static std::string name = SpecialIdentifier::Self;
        return name;
    }
}
