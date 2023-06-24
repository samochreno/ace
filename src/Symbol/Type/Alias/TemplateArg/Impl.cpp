#include "Symbol/Type/Alias/TemplateArg/Impl.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Type::Alias::TemplateArg
{
    Impl::Impl(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        Symbol::Type::IBase* const t_aliasedType,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_AliasedType{ t_aliasedType },
        m_Index{ t_index }
    {
    }

    auto Impl::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Impl::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AliasedType->GetSelfScope();
    }

    auto Impl::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Impl::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeAlias;
    }

    auto Impl::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Impl::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto Impl::CollectTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_AliasedType->CollectTemplateArgs();
    }

    auto Impl::CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_AliasedType->CollectImplTemplateArgs();
    }

    auto Impl::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return m_AliasedType->GetSizeKind();
    }

    auto Impl::SetAsUnsized() -> void
    {
        return m_AliasedType->SetAsUnsized();
    }

    auto Impl::SetAsPrimitivelyEmittable() -> void
    {
        m_AliasedType->SetAsPrimitivelyEmittable();
    }

    auto Impl::IsPrimitivelyEmittable() const -> bool
    {
        return m_AliasedType->IsPrimitivelyEmittable();
    }

    auto Impl::SetAsTriviallyCopyable() -> void
    {
        m_AliasedType->SetAsTriviallyCopyable();
    }

    auto Impl::IsTriviallyCopyable() const -> bool
    {
        return m_AliasedType->IsTriviallyCopyable();
    }

    auto Impl::SetAsTriviallyDroppable() -> void
    {
        m_AliasedType->SetAsTriviallyDroppable();
    }

    auto Impl::IsTriviallyDroppable() const -> bool
    {
        return m_AliasedType->IsTriviallyDroppable();
    }

    auto Impl::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateCopyGlueBody(t_glueSymbol);
    }

    auto Impl::CreateDropGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateDropGlueBody(t_glueSymbol);
    }

    auto Impl::BindCopyGlue(Symbol::Function* const t_glue) -> void
    {
        m_AliasedType->BindCopyGlue(t_glue);
    }

    auto Impl::GetCopyGlue() const -> std::optional<Symbol::Function*>
    {
        return m_AliasedType->GetCopyGlue();
    }

    auto Impl::BindDropGlue(Symbol::Function* const t_glue) -> void
    {
        m_AliasedType->BindDropGlue(t_glue);
    }

    auto Impl::GetDropGlue() const -> std::optional<Symbol::Function*>
    {
        return m_AliasedType->GetDropGlue();
    }

    auto Impl::GetAliasedType() const -> Symbol::Type::IBase*
    {
        return m_AliasedType;
    }

    auto Impl::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
