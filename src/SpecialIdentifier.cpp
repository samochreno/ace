#include "SpecialIdentifier.hpp"

#include <string>

#include "Symbol/Base.hpp"
#include "Asserts.hpp"

namespace Ace::SpecialIdentifier
{
    static uint64_t Count{};

    auto CreateAnonymous() -> std::string
    {
        return "$anonymous_" + std::to_string(Count++);
    }

    auto CreateTemplate(const std::string& t_templateName) -> std::string
    {
        return "$template_" + t_templateName;
    }

    auto IsReference(const std::string& t_identifier) -> bool
    {
        return t_identifier.starts_with("$reference_");
    }
}
