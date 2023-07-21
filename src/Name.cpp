#include "Name.hpp"

#include <vector>
#include <string>

#include "Assert.hpp"
#include "SpecialIdentifier.hpp"
#include "Compilation.hpp"
#include "Identifier.hpp"

namespace Ace
{
    SymbolName::SymbolName()
        : Sections{}, IsGlobal{}
    {
    }

    SymbolName::SymbolName(
        const SymbolNameSection& section,
        const SymbolNameResolutionScope& resolutionScope
    ) : Sections{},
        IsGlobal{ resolutionScope == SymbolNameResolutionScope::Global }
    {
        ACE_ASSERT(
            (resolutionScope == SymbolNameResolutionScope::Local) || 
            (resolutionScope == SymbolNameResolutionScope::Global)
        );

        Sections.push_back(section);
    }

    SymbolName::SymbolName(
        const std::vector<SymbolNameSection>& sections, 
        const SymbolNameResolutionScope& resolutionScope
    ) : Sections{ sections },
        IsGlobal{ resolutionScope == SymbolNameResolutionScope::Global }
    {
        ACE_ASSERT(
            (resolutionScope == SymbolNameResolutionScope::Local) || 
            (resolutionScope == SymbolNameResolutionScope::Global)
        );
    }

    SymbolName::~SymbolName()
    {
    }

    SymbolNameSection::SymbolNameSection()
        : Name{}, TemplateArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(const Identifier& name)
        : Name{ name }, TemplateArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(
        const Identifier& name,
        const std::vector<SymbolName>& templateArgs
    ) : Name{ name }, TemplateArgs{ templateArgs }
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
        const Ace::SymbolName& symbolName,
        const std::vector<TypeNameModifier>& modifiers
    ) : SymbolName{ symbolName },
        Modifiers{ modifiers }
    {
    }

    TypeName::~TypeName()
    {
    }

    static auto CreateModifierTypeFullyQualifiedName(
        const SourceLocation& sourceLocation,
        const TypeNameModifier modifier
    ) -> SymbolName
    {
        const auto& natives =
            sourceLocation.Buffer->GetCompilation()->Natives;

        switch (modifier)
        {
            case TypeNameModifier::Reference:
            {
                return natives->Reference.CreateFullyQualifiedName(
                    sourceLocation
                );
            }

            case TypeNameModifier::StrongPointer:
            {
                return natives->StrongPointer.CreateFullyQualifiedName(
                    sourceLocation
                );
            }

            case TypeNameModifier::WeakPointer:
            {
                return natives->WeakPointer.CreateFullyQualifiedName(
                    sourceLocation
                );
            }
        }
    }

    auto TypeName::ToSymbolName(
        const Compilation* const compilation
    ) const -> Ace::SymbolName
    {
        if (Modifiers.empty())
        {
            return SymbolName;
        }

        auto name = SymbolName;
        std::for_each(rbegin(Modifiers), rend(Modifiers),
        [&](const TypeNameModifier modifier)
        {
            const auto& firstNameSectionSourceLocation =
                SymbolName.Sections.front().Name.SourceLocation;
            const auto&  lastNameSectionSourceLocation =
                SymbolName.Sections.back().Name.SourceLocation;

            const SourceLocation sourceLocation
            {
                firstNameSectionSourceLocation.Buffer,
                firstNameSectionSourceLocation.CharacterBeginIterator,
                lastNameSectionSourceLocation.CharacterEndIterator,
            };

            auto modifiedName = CreateModifierTypeFullyQualifiedName(
                sourceLocation,
                modifier
            );
            modifiedName.Sections.back().TemplateArgs.push_back(name);
            name = modifiedName;
        });

        return name;
    }
}
