#include "Symbols/Types/StructTypeSymbol.hpp"

#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emittable.hpp"
#include "GlueGeneration.hpp"

namespace Ace
{
    StructTypeSymbol::StructTypeSymbol(
        const std::shared_ptr<Scope>& t_selfScope,
        const Identifier& t_name,
        const AccessModifier t_accessModifier
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier }
    {
    }

    auto StructTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto StructTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto StructTypeSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto StructTypeSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Struct;
    }

    auto StructTypeSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto StructTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto StructTypeSymbol::GetSizeKind() const -> Expected<TypeSizeKind>
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

            const auto variables = GetVars();

            const auto canResolveSize = TransformExpectedVector(GetVars(),
            [](const InstanceVarSymbol* const t_variable) -> Expected<void>
            {
                ACE_TRY(sizeKind, t_variable->GetType()->GetSizeKind());
                ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);
                return Void{};
            });
            
            m_IsResolvingSize = false;

            ACE_TRY_ASSERT(canResolveSize);
            return TypeSizeKind::Sized;
        }();

        m_OptSizeKindCache = expSizeKind;
        return expSizeKind;
    }

    auto StructTypeSymbol::SetAsUnsized() -> void
    {
        m_IsUnsized = true;
    }

    auto StructTypeSymbol::SetAsPrimitivelyEmittable() -> void
    {
        m_IsPrimitivelyEmittable = true;
    }

    auto StructTypeSymbol::IsPrimitivelyEmittable() const -> bool
    {
        return m_IsPrimitivelyEmittable;
    }

    auto StructTypeSymbol::SetAsTriviallyCopyable() -> void
    {
        m_IsTriviallyCopyable = true;
    }

    auto StructTypeSymbol::IsTriviallyCopyable() const -> bool
    {
        return m_IsTriviallyCopyable;
    }

    auto StructTypeSymbol::SetAsTriviallyDroppable() -> void
    {
        m_IsTriviallyDroppable = true;
    }

    auto StructTypeSymbol::IsTriviallyDroppable() const -> bool
    {
        return m_IsTriviallyDroppable;
    }

    auto StructTypeSymbol::CreateCopyGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>> 
    {
        return GlueGeneration::CreateCopyGlueBody(
            GetCompilation(),
            t_glueSymbol,
            this
        );
    }

    auto StructTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return GlueGeneration::CreateDropGlueBody(
            GetCompilation(),
            t_glueSymbol,
            this
        );
    }

    auto StructTypeSymbol::BindCopyGlue(FunctionSymbol* const t_symbol) -> void
    {
        ACE_ASSERT(t_symbol);
        m_OptCopyGlue = t_symbol;
    }

    auto StructTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_OptCopyGlue;
    }

    auto StructTypeSymbol::BindDropGlue(FunctionSymbol* const t_symbol) -> void
    {
        ACE_ASSERT(t_symbol);
        m_OptDropGlue = t_symbol;
    }

    auto StructTypeSymbol::GetDropGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_OptDropGlue;
    }

    auto StructTypeSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_SelfScope->CollectTemplateArgs();
    }

    auto StructTypeSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_SelfScope->CollectImplTemplateArgs();
    }

    auto StructTypeSymbol::GetVars() const -> std::vector<InstanceVarSymbol*>
    {
        auto variables = m_SelfScope->CollectSymbols<InstanceVarSymbol>();
        std::sort(begin(variables), end(variables),
        [](
            const InstanceVarSymbol* const t_lhs,
            const InstanceVarSymbol* const t_rhs
        )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        return variables;
    }
}
