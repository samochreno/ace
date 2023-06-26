#include "Symbols/Types/Aliases/TemplateArgs/ImplTemplateArgAliasTypeSymbol.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ImplTemplateArgAliasTypeSymbol::ImplTemplateArgAliasTypeSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        ITypeSymbol* const t_aliasedType,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_AliasedType{ t_aliasedType },
        m_Index{ t_index }
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

    auto ImplTemplateArgAliasTypeSymbol::GetName() const -> const std::string&
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
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateCopyGlueBody(t_glueSymbol);
    }

    auto ImplTemplateArgAliasTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateDropGlueBody(t_glueSymbol);
    }

    auto ImplTemplateArgAliasTypeSymbol::BindCopyGlue(
        FunctionSymbol* const t_glue
    ) -> void
    {
        m_AliasedType->BindCopyGlue(t_glue);
    }

    auto ImplTemplateArgAliasTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_AliasedType->GetCopyGlue();
    }

    auto ImplTemplateArgAliasTypeSymbol::BindDropGlue(
        FunctionSymbol* const t_glue
    ) -> void
    {
        m_AliasedType->BindDropGlue(t_glue);
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
