#include "Name.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "SpecialIdentifier.hpp"
#include "Compilation.hpp"

namespace Ace::Name
{
    namespace Symbol
    {
        Full::Full()
            : Sections{}, IsGlobal{}
        {
        }

        Full::Full(
            const Section& t_section,
            const bool& t_isGlobal
        ) : Sections{}, IsGlobal{ t_isGlobal }
        {
            Sections.push_back(t_section);
        }

        Full::Full(
            const std::vector<Section>& t_sections, 
            const bool& t_isGlobal
        ) : Sections{ t_sections }, IsGlobal{ t_isGlobal }
        {
        }

        Full::~Full()
        {
        }

        Section::Section()
            : Name{}, TemplateArguments{}
        {
        }

        Section::Section(const std::string& t_name)
            : Name{ t_name }, TemplateArguments{}
        {
        }

        Section::Section(
            const std::string& t_name,
            const std::vector<Name::Symbol::Full>& t_templateArguments
        ) : Name{ t_name }, TemplateArguments{ t_templateArguments }
        {
        }

        Section::~Section()
        {
        }
    }

    Type::Type()
        : SymbolName{}, Modifiers{}
    {
    }

    Type::Type(
        const Name::Symbol::Full& t_symbolName,
        const std::vector<TypeModifier>& t_modifiers
    ) : SymbolName{ t_symbolName },
        Modifiers{ t_modifiers }
    {
    }

    Type::~Type()
    {
    }

    static auto GetModifierFullyQualifiedName(
        const Compilation& t_compilation,
        const TypeModifier& t_modifier
    ) -> const Name::Symbol::Full&
    {
        const auto& natives = t_compilation.Natives;

        switch (t_modifier)
        {
            case TypeModifier::Reference:
            {
                return natives->Reference.GetFullyQualifiedName();
            }

            case TypeModifier::StrongPointer:
            {
                return natives->StrongPointer.GetFullyQualifiedName();
            }

            case TypeModifier::WeakPointer:
            {
                return natives->WeakPointer.GetFullyQualifiedName();
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    auto Type::ToSymbolName(
        const Compilation& t_compilation
    ) const -> Name::Symbol::Full
    {
        if (Modifiers.empty())
        {
            return SymbolName;
        }

        auto name = SymbolName;
        std::for_each(rbegin(Modifiers), rend(Modifiers),
        [&](const TypeModifier& t_modifier)
        {
            auto modifiedName = GetModifierFullyQualifiedName(
                t_compilation,
                t_modifier
            );
            modifiedName.Sections.back().TemplateArguments.push_back(name);
            name = modifiedName;
        });

        return name;
    }
}
