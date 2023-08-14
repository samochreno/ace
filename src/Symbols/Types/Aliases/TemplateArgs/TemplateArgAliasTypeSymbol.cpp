#include "Symbols/Types/Aliases/TemplateArgs/TemplateArgAliasTypeSymbol.hpp"

#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    auto ITemplateArgAliasTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::BindCopyGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        return dynamic_cast<const ISizedTypeSymbol*>(
            GetAliasedType()
        )->GetCopyGlue();
    }

    auto ITemplateArgAliasTypeSymbol::BindDropGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ITemplateArgAliasTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        return dynamic_cast<const ISizedTypeSymbol*>(
            GetAliasedType()
        )->GetDropGlue();
    }
}
