#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    NormalTemplateArgAliasTypeSymbol::NormalTemplateArgAliasTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const Identifier& name,
        ITypeSymbol* const aliasedType,
        const size_t index
    ) : m_Scope{ scope },
        m_Name{ name },
        m_AliasedType{ aliasedType },
        m_Index{ index }
    {
    }

    auto NormalTemplateArgAliasTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AliasedType->GetSelfScope();
    }

    auto NormalTemplateArgAliasTypeSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeAlias;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto NormalTemplateArgAliasTypeSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_AliasedType->CollectTemplateArgs();
    }

    auto NormalTemplateArgAliasTypeSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_AliasedType->CollectImplTemplateArgs();
    }

    auto NormalTemplateArgAliasTypeSymbol::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return m_AliasedType->GetSizeKind();
    }

    auto NormalTemplateArgAliasTypeSymbol::SetAsUnsized() -> void
    {
        return m_AliasedType->SetAsUnsized();
    }

    auto NormalTemplateArgAliasTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        m_AliasedType->SetAsPrimitivelyEmittable();
    }

    auto NormalTemplateArgAliasTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        return m_AliasedType->IsPrimitivelyEmittable();
    }

    auto NormalTemplateArgAliasTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        m_AliasedType->SetAsTriviallyCopyable();
    }

    auto NormalTemplateArgAliasTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        return m_AliasedType->IsTriviallyCopyable();
    }

    auto NormalTemplateArgAliasTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        m_AliasedType->SetAsTriviallyDroppable();
    }

    auto NormalTemplateArgAliasTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        return m_AliasedType->IsTriviallyDroppable();
    }

    auto NormalTemplateArgAliasTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateCopyGlueBody(glueSymbol);
    }

    auto NormalTemplateArgAliasTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return m_AliasedType->CreateDropGlueBody(glueSymbol);
    }

    auto NormalTemplateArgAliasTypeSymbol::BindCopyGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        m_AliasedType->BindCopyGlue(glue);
    }

    auto NormalTemplateArgAliasTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_AliasedType->GetCopyGlue();
    }

    auto NormalTemplateArgAliasTypeSymbol::BindDropGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        m_AliasedType->BindDropGlue(glue);
    }

    auto NormalTemplateArgAliasTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_AliasedType->GetDropGlue();
    }

    auto NormalTemplateArgAliasTypeSymbol::GetAliasedType() const -> ITypeSymbol*
    {
        return m_AliasedType;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
