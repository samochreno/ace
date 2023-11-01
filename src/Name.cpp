#include "Name.hpp"

#include <vector>
#include <string>

#include "Assert.hpp"
#include "Compilation.hpp"
#include "Ident.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    static auto CreateFirstSrcLocation(
        const SymbolName& symbolName
    ) -> SrcLocation
    {
        return symbolName.Sections.front().CreateSrcLocation();
    }

    SymbolNameSection::SymbolNameSection(
    ) : Name{},
        TypeArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(
        const Ident& name
    ) : Name{ name },
        TypeArgs{}
    {
    }

    SymbolNameSection::SymbolNameSection(
        const Ident& name,
        const std::vector<SymbolName>& typeArgs
    ) : Name{ name },
        TypeArgs{ typeArgs }
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
        if (!symbolNameSection.TypeArgs.empty())
        {
            return CreateLastSrcLocation(
                symbolNameSection.TypeArgs.back().Sections.back()
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
        const TypeNameModifier modifier,
        const SymbolName& modifiedName
    ) -> SymbolName
    {
        const auto& natives =
            srcLocation.Buffer->GetCompilation()->GetNatives();

        switch (modifier)
        {
            case TypeNameModifier::Ref:
            {
                auto name = natives.Ref.CreateFullyQualifiedName(srcLocation);
                name.Sections.back().TypeArgs.push_back(modifiedName);
                return name;
            }

            case TypeNameModifier::AutoStrongPtr:
            {
                auto name = natives.StrongPtr.CreateFullyQualifiedName(
                    srcLocation
                );
                name.Sections.back().TypeArgs.push_back(modifiedName);
                return name;
            }

            case TypeNameModifier::DynStrongPtr:
            {
                auto name = natives.DynStrongPtr.CreateFullyQualifiedName(
                    srcLocation
                );
                name.Sections.back().TypeArgs.push_back(modifiedName);
                return name;
            }

            case TypeNameModifier::WeakPtr:
            {
                auto name = natives.WeakPtr.CreateFullyQualifiedName(
                    srcLocation
                );

                name.Sections.back().TypeArgs.push_back(
                    natives.StrongPtr.CreateFullyQualifiedName(srcLocation)
                );
                name.Sections.back().TypeArgs.back().Sections.back().TypeArgs.push_back(
                    modifiedName
                );

                name.Sections.back().TypeArgs.push_back(
                    natives.Ptr.CreateFullyQualifiedName(srcLocation)
                );

                return name;
            }
        }
    }

    static auto ModifyTypeSymbol(
        ITypeSymbol* const typeSymbol,
        const TypeNameModifier modifier
    ) -> ITypeSymbol*
    {
        switch (modifier)
        {
            case TypeNameModifier::Ref:
            {
                return typeSymbol->GetWithRef();
            }

            case TypeNameModifier::AutoStrongPtr:
            {
                return typeSymbol->GetWithAutoStrongPtr();
            }

            case TypeNameModifier::DynStrongPtr:
            {
                return typeSymbol->GetWithDynStrongPtr();
            }
            
            case TypeNameModifier::WeakPtr:
            {
                return typeSymbol->GetWithWeakPtr();
            }
        }
    }

    auto ModifyTypeSymbol(
        ITypeSymbol* const pureTypeSymbol,
        std::vector<TypeNameModifier> modifiers
    ) -> ISizedTypeSymbol*
    {
        ACE_ASSERT(!modifiers.empty());

        ISizedTypeSymbol* typeSymbol = nullptr;
        while (!modifiers.empty())
        {
            const auto modifier = modifiers.back();

            auto* typeSymbolToModify = typeSymbol ?
                dynamic_cast<ISizedTypeSymbol*>(typeSymbol) :
                pureTypeSymbol;

            typeSymbol = dynamic_cast<ISizedTypeSymbol*>(ModifyTypeSymbol(
                typeSymbolToModify,
                modifier
            ));
            ACE_ASSERT(typeSymbol);

            modifiers.pop_back();
        }

        return typeSymbol;
    }
}
