#include "Name.hpp"

#include <vector>
#include <string>

#include "Assert.hpp"
#include "SpecialIdent.hpp"
#include "Compilation.hpp"
#include "Ident.hpp"

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

    SymbolNameSection::SymbolNameSection(const Ident& name)
        : Name{ name }, TemplateArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(
        const Ident& name,
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
        const SrcLocation& srcLocation,
        const TypeNameModifier modifier
    ) -> SymbolName
    {
        const auto& natives =
            srcLocation.Buffer->GetCompilation()->Natives;

        switch (modifier)
        {
            case TypeNameModifier::Ref:
            {
                return natives->Ref.CreateFullyQualifiedName(
                    srcLocation
                );
            }

            case TypeNameModifier::StrongPtr:
            {
                return natives->StrongPtr.CreateFullyQualifiedName(
                    srcLocation
                );
            }

            case TypeNameModifier::WeakPtr:
            {
                return natives->WeakPtr.CreateFullyQualifiedName(
                    srcLocation
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
            const auto& firstNameSectionSrcLocation =
                SymbolName.Sections.front().Name.SrcLocation;
            const auto&  lastNameSectionSrcLocation =
                SymbolName.Sections.back().Name.SrcLocation;

            const SrcLocation srcLocation
            {
                firstNameSectionSrcLocation.Buffer,
                firstNameSectionSrcLocation.CharacterBeginIterator,
                lastNameSectionSrcLocation.CharacterEndIterator,
            };

            auto modifiedName = CreateModifierTypeFullyQualifiedName(
                srcLocation,
                modifier
            );
            modifiedName.Sections.back().TemplateArgs.push_back(name);
            name = modifiedName;
        });

        return name;
    }
}
