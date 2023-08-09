#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ImplTemplateParamTypeSymbol::ImplTemplateParamTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_Scope{ scope },
        m_Name{ name }
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

    auto ImplTemplateParamTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ImplTemplateParamTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::ImplTemplateParam;
    }

    auto ImplTemplateParamTypeSymbol::GetCategory() const -> SymbolCategory
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
        return Expected{ TypeSizeKind::Sized, DiagnosticBag::Create() };
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
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::BindCopyGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::BindDropGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ImplTemplateParamTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }
}
