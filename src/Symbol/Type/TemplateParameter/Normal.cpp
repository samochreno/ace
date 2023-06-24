#include "Symbol/Type/TemplateParameter/Normal.hpp"

#include <vector>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Type::TemplateParameter
{
    Normal::Normal(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        ACE_UNREACHABLE();
    }

    auto Normal::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Normal::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplateParameter;
    }

    auto Normal::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Normal::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto Normal::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }

    auto Normal::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return {};
    }

    auto Normal::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return TypeSizeKind::Unsized;
    }

    auto Normal::SetAsUnsized() -> void
    {
    }

    auto Normal::SetAsPrimitivelyEmittable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto Normal::IsPrimitivelyEmittable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto Normal::SetAsTriviallyCopyable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto Normal::IsTriviallyCopyable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto Normal::SetAsTriviallyDroppable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto Normal::IsTriviallyDroppable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto Normal::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto Normal::CreateDropGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto Normal::BindCopyGlue(Symbol::Function* const t_glue) -> void
    {
        ACE_UNREACHABLE();
    }

    auto Normal::GetCopyGlue() const -> std::optional<Symbol::Function*>
    {
        ACE_UNREACHABLE();
    }

    auto Normal::BindDropGlue(Symbol::Function* const t_glue) -> void
    {
        ACE_UNREACHABLE();
    }

    auto Normal::GetDropGlue() const -> std::optional<Symbol::Function*>
    {
        ACE_UNREACHABLE();
    }
}
