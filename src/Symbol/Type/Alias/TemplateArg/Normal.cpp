#include "Symbol/Type/Alias/TemplateArg/Normal.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Type::Alias::TemplateArg
{
    Normal::Normal(
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

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AliasedType->GetSelfScope();
    }

    auto Normal::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Normal::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeAlias;
    }

    auto Normal::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Normal::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto Normal::CollectTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_AliasedType->CollectTemplateArgs();
    }

    auto Normal::CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_AliasedType->CollectImplTemplateArgs();
    }

    auto Normal::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return m_AliasedType->GetSizeKind();
    }

    auto Normal::SetAsUnsized() -> void
    {
        return m_AliasedType->SetAsUnsized();
    }

    auto Normal::SetAsPrimitivelyEmittable() -> void
    {
        m_AliasedType->SetAsPrimitivelyEmittable();
    }

    auto Normal::IsPrimitivelyEmittable() const -> bool
    {
        return m_AliasedType->IsPrimitivelyEmittable();
    }

    auto Normal::SetAsTriviallyCopyable() -> void
    {
        m_AliasedType->SetAsTriviallyCopyable();
    }

    auto Normal::IsTriviallyCopyable() const -> bool
    {
        return m_AliasedType->IsTriviallyCopyable();
    }

    auto Normal::SetAsTriviallyDroppable() -> void
    {
        m_AliasedType->SetAsTriviallyDroppable();
    }

    auto Normal::IsTriviallyDroppable() const -> bool
    {
        return m_AliasedType->IsTriviallyDroppable();
    }

    auto Normal::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateCopyGlueBody(t_glueSymbol);
    }

    auto Normal::CreateDropGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateDropGlueBody(t_glueSymbol);
    }

    auto Normal::BindCopyGlue(Symbol::Function* const t_glue) -> void
    {
        m_AliasedType->BindCopyGlue(t_glue);
    }

    auto Normal::GetCopyGlue() const -> std::optional<Symbol::Function*>
    {
        return m_AliasedType->GetCopyGlue();
    }

    auto Normal::BindDropGlue(Symbol::Function* const t_glue) -> void
    {
        m_AliasedType->BindDropGlue(t_glue);
    }

    auto Normal::GetDropGlue() const -> std::optional<Symbol::Function*>
    {
        return m_AliasedType->GetDropGlue();
    }

    auto Normal::GetAliasedType() const -> Symbol::Type::IBase*
    {
        return m_AliasedType;
    }

    auto Normal::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
