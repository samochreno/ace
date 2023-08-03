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
    auto ITypeSymbol::IsRef() const -> bool
    {
        auto* const self = GetUnaliased();

        const auto& ref = GetCompilation()->GetNatives()->Ref;

        if (self->GetScope() != ref.GetSymbol()->GetScope())
        {
            return false;
        }

        const auto refName = ref.CreateFullyQualifiedName(
            SrcLocation{}
        );
        if (self->GetName().String != refName.Sections.back().Name.String)
        {
            return false;
        }

        return true;
    }

    auto ITypeSymbol::GetWithoutRef() -> ITypeSymbol*
    {
        if (!IsRef())
        {
            return this;
        }

        auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front();
    }

    auto ITypeSymbol::GetWithoutRef() const -> const ITypeSymbol*
    {
        if (!IsRef())
        {
            return this;
        }

        const auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front()->GetUnaliased();
    }

    auto ITypeSymbol::GetWithRef() -> ITypeSymbol*
    {
        ACE_ASSERT(!IsRef());

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->Ref.GetSymbol(),
            std::nullopt,
            {},
            { this->GetUnaliased() }
        ).Unwrap();

        auto* const typeSymbol = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(typeSymbol);

        return typeSymbol;
    }

    auto ITypeSymbol::IsStrongPtr() const -> bool
    {
        auto* const self = GetUnaliased();

        const auto& strongPtr = GetCompilation()->GetNatives()->StrongPtr;

        if (self->GetScope() != strongPtr.GetSymbol()->GetScope())
        {
            return false;
        }

        const auto strongPtrName = strongPtr.CreateFullyQualifiedName(
            SrcLocation{}
        );
        if (self->GetName().String != strongPtrName.Sections.back().Name.String)
        {
            return false;
        }

        return true;
    }

    auto ITypeSymbol::GetWithoutStrongPtr() -> ITypeSymbol*
    {
        if (!IsStrongPtr())
        {
            return this;
        }

        auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front()->GetUnaliased();
    }

    auto ITypeSymbol::GetWithStrongPtr() -> ITypeSymbol*
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->StrongPtr.GetSymbol(),
            std::nullopt,
            {},
            { this->GetUnaliased() }
        ).Unwrap();

        auto* const typeSymbol = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(typeSymbol);

        return typeSymbol;
    }

    auto ITypeSymbol::IsWeakPtr() const -> bool
    {
        auto* const self = GetUnaliased();

        const auto& weakPtr = GetCompilation()->GetNatives()->WeakPtr;

        if (self->GetScope() != weakPtr.GetSymbol()->GetScope())
        {
            return false;
        }

        const auto weakPtrName = weakPtr.CreateFullyQualifiedName(
            SrcLocation{}
        );
        if (self->GetName().String != weakPtrName.Sections.back().Name.String)
        {
            return false;
        }

        return true;
    }

    auto ITypeSymbol::GetWithoutWeakPtr() -> ITypeSymbol*
    {
        if (!IsWeakPtr())
        {
            return this;
        }

        auto* const self = GetUnaliased();
        return self->CollectTemplateArgs().front();
    }

    auto ITypeSymbol::GetWithWeakPtr() -> ITypeSymbol*
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->WeakPtr.GetSymbol(),
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

        const Ident name
        {
            self->GetName().SrcLocation,
            SpecialIdent::CreateTemplate(self->GetName().String),
        };

        auto expTemplate =
            GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(name);
        
        return expTemplate ?
            std::optional{ expTemplate.Unwrap() } :
            std::nullopt;
    }
}
