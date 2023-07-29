#include "Symbols/Types/ErrorTypeSymbol.hpp"

namespace Ace
{
    ErrorTypeSymbol::ErrorTypeSymbol(
        const std::shared_ptr<Scope>& scope
    ) : m_Scope{ scope },
        m_SelfScope{ scope->GetOrCreateChild({}) },
        m_Name
        {
            GetCompilation()->PackageFileBuffer->CreateFirstLocation(),
            "<Error type>",
        }
    {
    }

    auto ErrorTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ErrorTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto ErrorTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ErrorTypeSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ErrorType;
    }

    auto ErrorTypeSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ErrorTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto ErrorTypeSymbol::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        return TypeSizeKind::Unsized;
    }

    auto ErrorTypeSymbol::SetAsUnsized() -> void
    {
    }

    auto ErrorTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::BindCopyGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::BindDropGlue(
        FunctionSymbol* const glue
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        ACE_UNREACHABLE();
    }

    auto ErrorTypeSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return {};
    }

    auto ErrorTypeSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return {};
    }
}
