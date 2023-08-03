#include "Name.hpp"

#include <vector>
#include <string>

#include "Assert.hpp"
#include "SpecialIdent.hpp"
#include "Compilation.hpp"
#include "Ident.hpp"

namespace Ace
{
    static auto CreateFirstSrcLocation(
        const SymbolName& symbolName
    ) -> SrcLocation
    {
        return symbolName.Sections.front().CreateSrcLocation();
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

    static auto CreateFirstSrcLocation(
        const SymbolNameSection& symbolNameSection
    ) -> SrcLocation
    {
        return symbolNameSection.Name.SrcLocation.CreateFirst();
    }

    static auto CreateLastSrcLocation(
        const SymbolNameSection& symbolNameSection
    ) -> SrcLocation
    {
        if (!symbolNameSection.TemplateArgs.empty())
        {
            return CreateLastSrcLocation(
                symbolNameSection.TemplateArgs.back().Sections.back()
            );
        }
        
        return symbolNameSection.Name.SrcLocation.CreateLast();
    }

    auto SymbolNameSection::CreateSrcLocation() const -> SrcLocation
    {
        return
        {
            CreateFirstSrcLocation(*this),
            CreateLastSrcLocation (*this),
        };
    }

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
    
    auto SymbolName::CreateSrcLocation() const -> SrcLocation
    {
        return
        {
            Sections.front().CreateSrcLocation(),
            Sections.back().CreateSrcLocation(),
        };
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
            srcLocation.Buffer->GetCompilation()->GetNatives();

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
        Compilation* const compilation
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
