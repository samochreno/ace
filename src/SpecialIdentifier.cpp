#include "SpecialIdentifier.hpp"

#include <string>

#include "Symbols/Symbol.hpp"
#include "Assert.hpp"

namespace Ace::SpecialIdentifier
{
    static uint64_t Count{};

    auto CreateAnonymous() -> std::string
    {
        return "$anonymous_" + std::to_string(Count++);
    }

    auto CreateTemplate(const std::string& templateName) -> std::string
    {
        return "$template_" + templateName;
    }

    auto CreateCopyGlue(const std::string& typePartialSignature) -> std::string
    {
        return "$copy_glue_" + typePartialSignature;
    }

    auto CreateDropGlue(const std::string& typePartialSignature) -> std::string
    {
        return "$drop_glue_" + typePartialSignature;
    }
}
