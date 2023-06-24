#include "Symbol/Type/Struct.hpp"

#include <vector>
#include <string>
#include <optional>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbol/Function.hpp"
#include "Diagnostics.hpp"
#include "Emittable.hpp"
#include "Core.hpp"

namespace Ace::Symbol::Type
{
    Struct::Struct(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const AccessModifier& t_accessModifier
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier }
    {
    }

    auto Struct::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Struct::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto Struct::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Struct::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Struct;
    }

    auto Struct::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Struct::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto Struct::GetSizeKind() const -> Expected<TypeSizeKind>
    {
        if (m_OptSizeKindCache.has_value())
        {
            return m_OptSizeKindCache.value();
        }

        const auto expSizeKind = [&]() -> Expected<TypeSizeKind>
        {
            // If size resolution is already in progress, we have detected a circular dependency.
            ACE_TRY_ASSERT(!m_IsResolvingSize);
            m_IsResolvingSize = true;

            if (m_IsPrimitivelyEmittable)
            {
                return TypeSizeKind::Sized;
            }

            if (m_IsUnsized)
            {
                return TypeSizeKind::Unsized;
            }

            const auto variables = GetVariables();

            const auto canResolveSize = TransformExpectedVector(GetVariables(),
            [](const Symbol::Variable::Normal::Instance* const t_variable) -> Expected<void>
            {
                ACE_TRY(sizeKind, t_variable->GetType()->GetSizeKind());
                ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);
                return Void;
            });
            
            m_IsResolvingSize = false;

            ACE_TRY_ASSERT(canResolveSize);
            return TypeSizeKind::Sized;
        }();

        m_OptSizeKindCache = expSizeKind;
        return expSizeKind;
    }

    auto Struct::SetAsUnsized() -> void
    {
        m_IsUnsized = true;
    }

    auto Struct::SetAsPrimitivelyEmittable() -> void
    {
        m_IsPrimitivelyEmittable = true;
    }

    auto Struct::IsPrimitivelyEmittable() const -> bool
    {
        return m_IsPrimitivelyEmittable;
    }

    auto Struct::SetAsTriviallyCopyable() -> void
    {
        m_IsTriviallyCopyable = true;
    }

    auto Struct::IsTriviallyCopyable() const -> bool
    {
        return m_IsTriviallyCopyable;
    }

    auto Struct::SetAsTriviallyDroppable() -> void
    {
        m_IsTriviallyDroppable = true;
    }

    auto Struct::IsTriviallyDroppable() const -> bool
    {
        return m_IsTriviallyDroppable;
    }

    auto Struct::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>> 
    {
        return Core::CreateCopyGlueBody(
            GetCompilation(),
            this,
            t_glueSymbol
        );
    }

    auto Struct::CreateDropGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return Core::CreateDropGlueBody(
            GetCompilation(),
            this,
            t_glueSymbol
        );
    }

    auto Struct::BindCopyGlue(Symbol::Function* const t_symbol) -> void
    {
        ACE_ASSERT(t_symbol);
        m_OptCopyGlue = t_symbol;
    }

    auto Struct::GetCopyGlue() const -> std::optional<Symbol::Function*>
    {
        return m_OptCopyGlue;
    }

    auto Struct::BindDropGlue(Symbol::Function* const t_symbol) -> void
    {
        ACE_ASSERT(t_symbol);
        m_OptDropGlue = t_symbol;
    }

    auto Struct::GetDropGlue() const -> std::optional<Symbol::Function*>
    {
        return m_OptDropGlue;
    }

    auto Struct::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_SelfScope->CollectTemplateArguments();
    }

    auto Struct::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_SelfScope->CollectImplTemplateArguments();
    }

    auto Struct::GetVariables() const -> std::vector<Symbol::Variable::Normal::Instance*>
    {
        auto variables = m_SelfScope->CollectSymbols<Symbol::Variable::Normal::Instance>();
        std::sort(begin(variables), end(variables),
        [](
            const Symbol::Variable::Normal::Instance* const t_lhs,
            const Symbol::Variable::Normal::Instance* const t_rhs
        )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        return variables;
    }
}
