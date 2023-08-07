#include "Symbols/Types/StructTypeSymbol.hpp"

#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Emittable.hpp"
#include "GlueGeneration.hpp"

namespace Ace
{
    StructTypeSymbol::StructTypeSymbol(
        const std::shared_ptr<Scope>& selfScope,
        const Ident& name,
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

    auto StructTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto StructTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::Struct;
    }

    auto StructTypeSymbol::GetCategory() const -> SymbolCategory
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

        const auto sizeKind = [&]() -> Expected<TypeSizeKind>
        {
            DiagnosticBag diagnostics{};

            if (m_ResolvingVar.has_value())
            {
                return diagnostics.Add(CreateStructVarCausesCycleError(
                    m_ResolvingVar.value()
                ));
            }

            if (m_IsPrimitivelyEmittable)
            {
                return Expected{ TypeSizeKind::Sized, diagnostics };
            }

            if (m_IsUnsized)
            {
                return Expected{ TypeSizeKind::Unsized, diagnostics };
            }

            const auto vars = CollectVars();
            const auto unsizedVarIt = std::find_if_not(
                begin(vars),
                end  (vars),
                [&](InstanceVarSymbol* const var)
                {
                    m_ResolvingVar = var;

                    const auto optSizeKind = diagnostics.Collect(
                        var->GetType()->GetSizeKind()
                    );
                    if (!optSizeKind.has_value())
                    {
                        return false;
                    }

                    if (optSizeKind.value() == TypeSizeKind::Unsized)
                    {
                        return false;
                    }

                    return true;
                }
            );

            m_ResolvingVar = std::nullopt;

            if (unsizedVarIt != end(vars))
            {
                return diagnostics;
            }

            return TypeSizeKind::Sized;
        }();

        m_OptSizeKindCache = sizeKind;
        return sizeKind;
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

    auto StructTypeSymbol::CollectVars() const -> std::vector<InstanceVarSymbol*>
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
