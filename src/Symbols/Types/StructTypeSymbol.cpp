#include "Symbols/Types/StructTypeSymbol.hpp"

#include <memory>
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

    auto StructTypeSymbol::DiagnoseCycle() const -> Diagnosed<void>
    {
        if (m_OptCycleDiagnosticsCache.has_value())
        {
            return Diagnosed<void>{ m_OptCycleDiagnosticsCache.value() };
        }

        const auto diagnostics = [&]() -> DiagnosticBag
        {
            if (m_IsPrimitivelyEmittable)
            {
                return DiagnosticBag::Create();
            }

            auto diagnostics = DiagnosticBag::Create();

            if (m_ResolvingVar.has_value())
            {
                diagnostics.Add(CreateStructVarCausesCycleError(
                    m_ResolvingVar.value()
                ));
                return std::move(diagnostics);
            }

            const auto vars = CollectVars();
            std::for_each(begin(vars), end(vars),
            [&](InstanceVarSymbol* const var)
            {
                m_ResolvingVar = var;
                (void)var->GetType()->DiagnoseCycle();
            });

            m_ResolvingVar = std::nullopt;
            return std::move(diagnostics);
        }();

        m_OptCycleDiagnosticsCache = diagnostics;
        return Diagnosed<void>{ diagnostics };
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
