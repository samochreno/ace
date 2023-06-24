#include "Symbol/Type/TemplateParam/Impl.hpp"

#include <string>
#include <vector>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Type::TemplateParam
{
    Impl::Impl(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto Impl::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Impl::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        ACE_UNREACHABLE();
    }

    auto Impl::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Impl::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ImplTemplateParam;
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
        return {};
    }

    auto Impl::CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }

    auto Impl::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return TypeSizeKind::Unsized;
    }

    auto Impl::SetAsUnsized() -> void
    {
    }

    auto Impl::SetAsPrimitivelyEmittable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto Impl::IsPrimitivelyEmittable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto Impl::SetAsTriviallyCopyable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto Impl::IsTriviallyCopyable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto Impl::SetAsTriviallyDroppable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto Impl::IsTriviallyDroppable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto Impl::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto Impl::CreateDropGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto Impl::BindCopyGlue(Symbol::Function* const t_glue) -> void
    {
        ACE_UNREACHABLE();
    }

    auto Impl::GetCopyGlue() const -> std::optional<Symbol::Function*>
    {
        ACE_UNREACHABLE();
    }

    auto Impl::BindDropGlue(Symbol::Function* const t_glue) -> void
    {
        ACE_UNREACHABLE();
    }

    auto Impl::GetDropGlue() const -> std::optional<Symbol::Function*>
    {
        ACE_UNREACHABLE();
    }
}
