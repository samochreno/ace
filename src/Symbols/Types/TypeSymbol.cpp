#include "Symbols/Types/TypeSymbol.hpp"

#include <vector>
#include <optional>

#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto ITypeSymbol::IsReference() const -> bool
    {
        auto* const self = GetUnaliased();

        const auto& reference = GetCompilation()->Natives->Reference;

        if (
            self->GetScope() !=
            reference.GetSymbol()->GetScope()
            )
        {
            return false;
        }

        const auto referenceName = reference.CreateFullyQualifiedName(
            SourceLocation{}
        );
        if (
            self->GetName().String !=
            referenceName.Sections.back().Name.String
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
        return self->CollectTemplateArgs().front()->GetUnaliased();
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

        const auto& strongPointer = GetCompilation()->Natives->StrongPointer;

        if (
            self->GetScope() !=
            strongPointer.GetSymbol()->GetScope()
            )
        {
            return false;
        }

        const auto strongPointerName = strongPointer.CreateFullyQualifiedName(
            SourceLocation{}
        );
        if (
            self->GetName().String !=
            strongPointerName.Sections.back().Name.String
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
        return self->CollectTemplateArgs().front()->GetUnaliased();
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

    auto ITypeSymbol::IsWeakPointer() const -> bool
    {
        auto* const self = GetUnaliased();

        const auto& weakPointer = GetCompilation()->Natives->WeakPointer;

        if (
            self->GetScope() !=
            weakPointer.GetSymbol()->GetScope()
            )
        {
            return false;
        }

        const auto weakPointerName = weakPointer.CreateFullyQualifiedName(
            SourceLocation{}
        );
        if (
            self->GetName().String !=
            weakPointerName.Sections.back().Name.String
            )
        {
            return false;
        }

        return true;
    }

    auto ITypeSymbol::GetWithoutWeakPointer() -> ITypeSymbol*
    {
        if (!IsWeakPointer())
        {
            return this;
        }

        auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front();
    }

    auto ITypeSymbol::GetWithWeakPointer() -> ITypeSymbol*
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->WeakPointer.GetSymbol(),
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

        const Identifier name
        {
            self->GetName().SourceLocation,
            SpecialIdentifier::CreateTemplate(self->GetName().String),
        };

        auto expTemplate =
            GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(name);
        
        return expTemplate ?
            expTemplate.Unwrap() :
            std::optional<TypeTemplateSymbol*>{};
    }
}
