#include "SpecialIdentifier.hpp"

#include <string>

#include "Symbols/Symbol.hpp"
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

    auto CreateCopyGlue(const std::string& t_typePartialSignature) -> std::string
    {
        return "$copy_glue_" + t_typePartialSignature;
    }

    auto CreateDropGlue(const std::string& t_typePartialSignature) -> std::string
    {
        return "$drop_glue_" + t_typePartialSignature;
    }
}
