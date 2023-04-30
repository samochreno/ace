#include "Symbol/Variable/Parameter/Self.hpp"

#include <string>

#include "SpecialIdentifier.hpp"

namespace Ace::Symbol::Variable::Parameter
{
    auto Self::GetName() const -> const std::string&
    {
        static std::string name = SpecialIdentifier::Self;
        return name;
    }
}
