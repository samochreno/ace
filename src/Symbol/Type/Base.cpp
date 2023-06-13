#include "Symbol/Type/Base.hpp"

#include <vector>
#include <optional>
#include <map>

#include "Symbol/Type/Alias/Base.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Template/Type.hpp"
#include "Scope.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "Utility.hpp"

namespace Ace::Symbol::Type
{
    auto IBase::IsReference() const -> bool
    {
        auto* const self = GetUnaliased();

        if (
            self->GetScope() !=
            GetCompilation()->Natives->Reference.GetSymbol()->GetScope()
            )
            return false;

        if (
            self->GetName() !=
            GetCompilation()->Natives->Reference.GetFullyQualifiedName().Sections.back().Name
            )
            return false;

        return true;
    }

    auto IBase::GetWithoutReference() -> Symbol::Type::IBase*
    {
        if (!IsReference())
            return this;

        auto* const self = GetUnaliased();
        return self->CollectTemplateArguments().front();
    }

    auto IBase::GetWithoutReference() const -> const Symbol::Type::IBase*
    {
        if (!IsReference())
            return this;

        const auto* const self = GetUnaliased();
        return self->CollectTemplateArguments().front();
    }

    auto IBase::GetWithReference() -> Symbol::Type::IBase*
    {
        ACE_ASSERT(!IsReference());

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->Reference.GetSymbol(),
            std::nullopt,
            {},
            { this->GetUnaliased() }
        ).Unwrap();

        auto* const typeSymbol = dynamic_cast<Symbol::Type::IBase*>(symbol);
        ACE_ASSERT(typeSymbol);

        return typeSymbol;
    }

    auto IBase::IsStrongPointer() const -> bool
    {
        auto* const self = GetUnaliased();

        if (
            self->GetScope() !=
            GetCompilation()->Natives->StrongPointer.GetSymbol()->GetScope())
            return false;

        if (
            self->GetName() !=
            GetCompilation()->Natives->StrongPointer.GetFullyQualifiedName().Sections.back().Name
            )
            return false;

        return true;
    }

    auto IBase::GetWithoutStrongPointer() -> Symbol::Type::IBase*
    {
        if (!IsStrongPointer())
            return this;

        auto* const self = GetUnaliased();
        return self->CollectTemplateArguments().front();
    }

    auto IBase::GetWithStrongPointer() -> Symbol::Type::IBase*
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer.GetSymbol(),
            std::nullopt,
            {},
            { this->GetUnaliased() }
        ).Unwrap();

        auto* const typeSymbol = dynamic_cast<Symbol::Type::IBase*>(symbol);
        ACE_ASSERT(typeSymbol);

        return typeSymbol;
    }

    auto IBase::GetUnaliased() -> Symbol::Type::IBase*
    {
        Symbol::Type::IBase* self = this;
        while (auto* aliasType = dynamic_cast<Symbol::Type::Alias::IBase*>(self))
        {
            self = aliasType->GetAliasedType();
        }

        return self;
    }

    auto IBase::GetUnaliased() const -> const Symbol::Type::IBase*
    {
        const Symbol::Type::IBase* self = this;
        while (auto* aliasType = dynamic_cast<const Symbol::Type::Alias::IBase*>(self))
        {
            self = aliasType->GetAliasedType();
        }

        return self;
    }

    auto IBase::GetTemplate() const -> std::optional<Symbol::Template::Type*>
    {
        auto* const self = GetUnaliased();

        auto expTemplate = GetScope()->ResolveStaticSymbol<Symbol::Template::Type>(
            SpecialIdentifier::CreateTemplate(self->GetName())
        );
        
        return expTemplate ? expTemplate.Unwrap() : std::optional<Symbol::Template::Type*>{};
    }
}
