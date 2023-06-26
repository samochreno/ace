#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

#include <vector>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    NormalTemplateParamTypeSymbol::NormalTemplateParamTypeSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto NormalTemplateParamTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalTemplateParamTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto NormalTemplateParamTypeSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplateParam;
    }

    auto NormalTemplateParamTypeSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto NormalTemplateParamTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto NormalTemplateParamTypeSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return {};
    }

    auto NormalTemplateParamTypeSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return {};
    }

    auto NormalTemplateParamTypeSymbol::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return TypeSizeKind::Unsized;
    }

    auto NormalTemplateParamTypeSymbol::SetAsUnsized() -> void
    {
    }

    auto NormalTemplateParamTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::BindCopyGlue(
        FunctionSymbol* const t_glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::BindDropGlue(
        FunctionSymbol* const t_glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto NormalTemplateParamTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }
}
