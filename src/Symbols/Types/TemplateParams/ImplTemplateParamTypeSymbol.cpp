#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ImplTemplateParamTypeSymbol::ImplTemplateParamTypeSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto ImplTemplateParamTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplTemplateParamTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto ImplTemplateParamTypeSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ImplTemplateParam;
    }

    auto ImplTemplateParamTypeSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ImplTemplateParamTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto ImplTemplateParamTypeSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return {};
    }

    auto ImplTemplateParamTypeSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return {};
    }

    auto ImplTemplateParamTypeSymbol::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return TypeSizeKind::Unsized;
    }

    auto ImplTemplateParamTypeSymbol::SetAsUnsized() -> void
    {
    }

    auto ImplTemplateParamTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::BindCopyGlue(
        FunctionSymbol* const t_glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::BindDropGlue(
        FunctionSymbol* const t_glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }
}
