#include "Symbols/Types/Aliases/TemplateArgs/ImplTemplateArgAliasTypeSymbol.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ImplTemplateArgAliasTypeSymbol::ImplTemplateArgAliasTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        ITypeSymbol* const aliasedType,
        const size_t index
    ) : m_Scope{ scope },
        m_Name{ name },
        m_AliasedType{ aliasedType },
        m_Index{ index }
    {
    }

    auto ImplTemplateArgAliasTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AliasedType->GetSelfScope();
    }

    auto ImplTemplateArgAliasTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeAlias;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto ImplTemplateArgAliasTypeSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_AliasedType->CollectTemplateArgs();
    }

    auto ImplTemplateArgAliasTypeSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_AliasedType->CollectImplTemplateArgs();
    }

    auto ImplTemplateArgAliasTypeSymbol::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return m_AliasedType->GetSizeKind();
    }

    auto ImplTemplateArgAliasTypeSymbol::SetAsUnsized() -> void
    {
        return m_AliasedType->SetAsUnsized();
    }

    auto ImplTemplateArgAliasTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        m_AliasedType->SetAsPrimitivelyEmittable();
    }

    auto ImplTemplateArgAliasTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        return m_AliasedType->IsPrimitivelyEmittable();
    }

    auto ImplTemplateArgAliasTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        m_AliasedType->SetAsTriviallyCopyable();
    }

    auto ImplTemplateArgAliasTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        return m_AliasedType->IsTriviallyCopyable();
    }

    auto ImplTemplateArgAliasTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        m_AliasedType->SetAsTriviallyDroppable();
    }

    auto ImplTemplateArgAliasTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        return m_AliasedType->IsTriviallyDroppable();
    }

    auto ImplTemplateArgAliasTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateCopyGlueBody(glueSymbol);
    }

    auto ImplTemplateArgAliasTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateDropGlueBody(glueSymbol);
    }

    auto ImplTemplateArgAliasTypeSymbol::BindCopyGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        m_AliasedType->BindCopyGlue(glue);
    }

    auto ImplTemplateArgAliasTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_AliasedType->GetCopyGlue();
    }

    auto ImplTemplateArgAliasTypeSymbol::BindDropGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        m_AliasedType->BindDropGlue(glue);
    }

    auto ImplTemplateArgAliasTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_AliasedType->GetDropGlue();
    }

    auto ImplTemplateArgAliasTypeSymbol::GetAliasedType() const -> ITypeSymbol*
    {
        return m_AliasedType;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
