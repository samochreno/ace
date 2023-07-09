#include "Symbols/Types/TypeSymbol.hpp"

#include <vector>
#include <optional>

#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "Diagnostics.hpp"
#include "Utility.hpp"

namespace Ace
{
    auto ITypeSymbol::IsReference() const -> bool
    {
        auto* const self = GetUnaliased();

        if (
            self->GetScope() !=
            GetCompilation()->Natives->Reference.GetSymbol()->GetScope()
            )
        {
            return false;
        }

        if (
            self->GetName() !=
            GetCompilation()->Natives->Reference.GetFullyQualifiedName().Sections.back().Name
            )
        {
            return false;
        }

        return true;
    }

    auto ITypeSymbol::GetWithoutReference() -> ITypeSymbol*
    {
        if (!IsReference())
        {
            return this;
        }

        auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front();
    }

    auto ITypeSymbol::GetWithoutReference() const -> const ITypeSymbol*
    {
        if (!IsReference())
        {
            return this;
        }

        const auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front();
    }

    auto ITypeSymbol::GetWithReference() -> ITypeSymbol*
    {
        ACE_ASSERT(!IsReference());

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->Reference.GetSymbol(),
            std::nullopt,
            {},
            { this->GetUnaliased() }
        ).Unwrap();

        auto* const typeSymbol = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(typeSymbol);

        return typeSymbol;
    }

    auto ITypeSymbol::IsStrongPointer() const -> bool
    {
        auto* const self = GetUnaliased();

        if (
            self->GetScope() !=
            GetCompilation()->Natives->StrongPointer.GetSymbol()->GetScope()
            )
        {
            return false;
        }

        if (
            self->GetName() !=
            GetCompilation()->Natives->StrongPointer.GetFullyQualifiedName().Sections.back().Name
            )
        {
            return false;
        }

        return true;
    }

    auto ITypeSymbol::GetWithoutStrongPointer() -> ITypeSymbol*
    {
        if (!IsStrongPointer())
        {
            return this;
        }

        auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front();
    }

    auto ITypeSymbol::GetWithStrongPointer() -> ITypeSymbol*
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer.GetSymbol(),
            std::nullopt,
            {},
            { this->GetUnaliased() }
        ).Unwrap();

        auto* const typeSymbol = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(typeSymbol);

        return typeSymbol;
    }

    auto ITypeSymbol::GetUnaliased() -> ITypeSymbol*
    {
        ITypeSymbol* self = this;
        while (auto* const aliasType = dynamic_cast<IAliasTypeSymbol*>(self))
        {
            self = aliasType->GetAliasedType();
        }

        return self;
    }

    auto ITypeSymbol::GetUnaliased() const -> const ITypeSymbol*
    {
        const ITypeSymbol* self = this;
        while (auto* const aliasType = dynamic_cast<const IAliasTypeSymbol*>(self))
        {
            self = aliasType->GetAliasedType();
        }

        return self;
    }

    auto ITypeSymbol::GetTemplate() const -> std::optional<TypeTemplateSymbol*>
    {
        auto* const self = GetUnaliased();

        auto expTemplate = GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(
            SpecialIdentifier::CreateTemplate(self->GetName())
        );
        
        return expTemplate ? expTemplate.Unwrap() : std::optional<TypeTemplateSymbol*>{};
    }
}
