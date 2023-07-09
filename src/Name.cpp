#include "Name.hpp"

#include <vector>
#include <string>

#include "Assert.hpp"
#include "SpecialIdentifier.hpp"
#include "Compilation.hpp"

namespace Ace
{
    SymbolName::SymbolName()
        : Sections{}, IsGlobal{}
    {
    }

    SymbolName::SymbolName(
        const SymbolNameSection& t_section,
        const SymbolNameResolutionScope& t_resolutionScope
    ) : Sections{},
        IsGlobal{ t_resolutionScope == SymbolNameResolutionScope::Global }
    {
        ACE_ASSERT(
            (t_resolutionScope == SymbolNameResolutionScope::Local) || 
            (t_resolutionScope == SymbolNameResolutionScope::Global)
        );

        Sections.push_back(t_section);
    }

    SymbolName::SymbolName(
        const std::vector<SymbolNameSection>& t_sections, 
        const SymbolNameResolutionScope& t_resolutionScope
    ) : Sections{ t_sections },
        IsGlobal{ t_resolutionScope == SymbolNameResolutionScope::Global }
    {
        ACE_ASSERT(
            (t_resolutionScope == SymbolNameResolutionScope::Local) || 
            (t_resolutionScope == SymbolNameResolutionScope::Global)
        );
    }

    SymbolName::~SymbolName()
    {
    }

    SymbolNameSection::SymbolNameSection()
        : Name{}, TemplateArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(const std::string& t_name)
        : Name{ t_name }, TemplateArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(
        const std::string& t_name,
        const std::vector<SymbolName>& t_templateArgs
    ) : Name{ t_name }, TemplateArgs{ t_templateArgs }
    {
    }

    SymbolNameSection::~SymbolNameSection()
    {
    }

    TypeName::TypeName()
        : SymbolName{}, Modifiers{}
    {
    }

    TypeName::TypeName(
        const Ace::SymbolName& t_symbolName,
        const std::vector<TypeNameModifier>& t_modifiers
    ) : SymbolName{ t_symbolName },
        Modifiers{ t_modifiers }
    {
    }

    TypeName::~TypeName()
    {
    }

    static auto GetModifierFullyQualifiedName(
        const Compilation* const t_compilation,
        const TypeNameModifier t_modifier
    ) -> const SymbolName&
    {
        const auto& natives = t_compilation->Natives;

        switch (t_modifier)
        {
            case TypeNameModifier::Reference:
            {
                return natives->Reference.GetFullyQualifiedName();
            }

            case TypeNameModifier::StrongPointer:
            {
                return natives->StrongPointer.GetFullyQualifiedName();
            }

            case TypeNameModifier::WeakPointer:
            {
                return natives->WeakPointer.GetFullyQualifiedName();
            }
        }
    }

    auto TypeName::ToSymbolName(
        const Compilation* const t_compilation
    ) const -> Ace::SymbolName
    {
        if (Modifiers.empty())
        {
            return SymbolName;
        }

        auto name = SymbolName;
        std::for_each(rbegin(Modifiers), rend(Modifiers),
        [&](const TypeNameModifier t_modifier)
        {
            auto modifiedName = GetModifierFullyQualifiedName(
                t_compilation,
                t_modifier
            );
            modifiedName.Sections.back().TemplateArgs.push_back(name);
            name = modifiedName;
        });

        return name;
    }
}
