#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Symbols/Types/EmittableTypeSymbol.hpp"
#include "Symbols/Types/NominalTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "Diagnostic.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class StructTypeSymbol :
        public virtual IConcreteTypeSymbol,
        public virtual INominalTypeSymbol
    {
    public:
        StructTypeSymbol(
            const std::shared_ptr<Scope>& bodyScope,
            const AccessModifier accessModifier,
            const Ident& name,
            const std::vector<ITypeSymbol*>& typeArgs
        );
        virtual ~StructTypeSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto DiagnoseCycle() const -> Diagnosed<void> final;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;

        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final;
        auto IsTriviallyCopyable() const -> bool final;
        auto SetAsTriviallyDroppable() -> void final;
        auto IsTriviallyDroppable() const -> bool final;

        auto CreateCopyGlueBlock(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;
        auto CreateDropGlueBlock(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;

        auto BindCopyGlue(FunctionSymbol* const glue) -> void final;
        auto GetCopyGlue() const -> std::optional<FunctionSymbol*> final;
        auto BindDropGlue(FunctionSymbol* const glue) -> void final;
        auto GetDropGlue() const -> std::optional<FunctionSymbol*> final;

        auto CollectFields() const -> std::vector<FieldVarSymbol*>;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        std::vector<ITypeSymbol*> m_TypeArgs{};

        mutable std::optional<FieldVarSymbol*> m_ResolvingField{};
        mutable std::optional<DiagnosticBag> m_OptCycleDiagnosticsCache{};
        bool m_IsPrimitivelyEmittable{};
        
        bool m_IsTriviallyCopyable{};
        bool m_IsTriviallyDroppable{};

        std::optional<FunctionSymbol*> m_OptCopyGlue{};
        std::optional<FunctionSymbol*> m_OptDropGlue{};
    };
}
