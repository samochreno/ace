#include "Symbols/Types/StructTypeSymbol.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "Noun.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Emittable.hpp"
#include "GlueGeneration.hpp"

namespace Ace
{
    StructTypeSymbol::StructTypeSymbol(
        const std::shared_ptr<Scope>& bodyScope,
        const AccessModifier accessModifier,
        const Ident& name,
        const std::vector<ITypeSymbol*>& typeArgs
    ) : m_BodyScope{ bodyScope },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_TypeArgs{ typeArgs }
    {
    }

    auto StructTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "struct" };
    }

    auto StructTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto StructTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto StructTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto StructTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto StructTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<StructTypeSymbol>(
            scope->CreateChild(),
            GetAccessModifier(),
            GetName(),
            context.TypeArgs
        );
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

            if (m_ResolvingField.has_value())
            {
                diagnostics.Add(CreateStructFieldCausesCycleError(
                    m_ResolvingField.value()
                ));
                return std::move(diagnostics);
            }

            const auto fields = CollectFields();
            std::for_each(begin(fields), end(fields),
            [&](FieldVarSymbol* const field)
            {
                m_ResolvingField = field;
                (void)field->GetType()->DiagnoseCycle();
            });

            m_ResolvingField = std::nullopt;
            return std::move(diagnostics);
        }();

        m_OptCycleDiagnosticsCache = diagnostics;
        return Diagnosed<void>{ diagnostics };
    }

    auto StructTypeSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        m_BodyScope = scope;
    }

    auto StructTypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        return m_TypeArgs;
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

    auto StructTypeSymbol::CreateCopyGlueBlock(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>> 
    {
        return GlueGeneration::CreateCopyGlueBlock(
            GetCompilation(),
            glueSymbol,
            this
        );
    }

    auto StructTypeSymbol::CreateDropGlueBlock(
        FunctionSymbol* const glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return GlueGeneration::CreateDropGlueBlock(
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

    auto StructTypeSymbol::CollectFields() const -> std::vector<FieldVarSymbol*>
    {
        auto fields = m_BodyScope->CollectSymbols<FieldVarSymbol>();
        std::sort(begin(fields), end(fields),
        [](FieldVarSymbol* const lhs, FieldVarSymbol* const rhs)
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        return fields;
    }
}
