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
        const std::shared_ptr<Scope>& selfScope,
        const Identifier& name,
        const AccessModifier accessModifier
    ) : m_SelfScope{ selfScope },
        m_Name{ name },
        m_AccessModifier{ accessModifier }
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
            // If size resolution is already in progress,
            // we have detected a circular dependency
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

            const auto vars = GetVars();

            const auto canResolveSize = TransformExpectedVector(GetVars(),
            [](const InstanceVarSymbol* const var) -> Expected<void>
            {
                ACE_TRY(sizeKind, var->GetType()->GetSizeKind());
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
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>> 
    {
        return GlueGeneration::CreateCopyGlueBody(
            GetCompilation(),
            glueSymbol,
            this
        );
    }

    auto StructTypeSymbol::CreateDropGlueBody(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return GlueGeneration::CreateDropGlueBody(
            GetCompilation(),
            glueSymbol,
            this
        );
    }

    auto StructTypeSymbol::BindCopyGlue(FunctionSymbol* const symbol) -> void
    {
        ACE_ASSERT(symbol);
        m_OptCopyGlue = symbol;
    }

    auto StructTypeSymbol::GetCopyGlue() const -> std::optional<FunctionSymbol*>
    {
        return m_OptCopyGlue;
    }

    auto StructTypeSymbol::BindDropGlue(FunctionSymbol* const symbol) -> void
    {
        ACE_ASSERT(symbol);
        m_OptDropGlue = symbol;
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
        auto vars = m_SelfScope->CollectSymbols<InstanceVarSymbol>();
        std::sort(begin(vars), end(vars),
        [](
            const InstanceVarSymbol* const lhs,
            const InstanceVarSymbol* const rhs
        )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        return vars;
    }
}
