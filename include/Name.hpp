#pragma once

#include <vector>
#include <string>

namespace Ace::Name
{
    namespace Symbol
    {
        struct Full;

        struct Section
        {
            Section();
            Section(const std::string& t_name);
            Section(const std::string& t_name, const std::vector<Name::Symbol::Full>& t_templateArguments);
            ~Section();

            std::string Name;
            std::vector<Name::Symbol::Full> TemplateArguments;
        };

        struct Full
        {
            Full();
            Full(const Section& t_section, const bool& t_isGlobal);
            Full(const std::vector<Section>& t_sections, const bool& t_isGlobal);
            ~Full();

            std::vector<Section> Sections;
            bool IsGlobal;
        };
    }

    enum class TypeModifier
    {
        Reference,
        StrongPointer,
        WeakPointer,
    };

    struct Type
    {
        Type();
        Type(
            const Name::Symbol::Full& t_symbolName,
            const std::vector<TypeModifier>& t_modifiers
        );
        ~Type();

        auto ToSymbolName() const -> Name::Symbol::Full;

        Name::Symbol::Full SymbolName;
        std::vector<TypeModifier> Modifiers;
    };
}
