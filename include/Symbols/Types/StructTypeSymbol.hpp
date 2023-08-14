#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class StructTypeSymbol : public virtual ISizedTypeSymbol
    {
    public:
        StructTypeSymbol(
            const std::shared_ptr<Scope>& selfScope,
            const Ident& name,
            const AccessModifier accessModifier
        );
        virtual ~StructTypeSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto DiagnoseCycle() const -> Diagnosed<void> final;

        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final;
        auto IsTriviallyCopyable() const -> bool final;
        auto SetAsTriviallyDroppable() -> void final;
        auto IsTriviallyDroppable() const -> bool final;

        auto CreateCopyGlueBody(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;
        auto CreateDropGlueBody(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;

        auto BindCopyGlue(FunctionSymbol* const glue) -> void final;
        auto GetCopyGlue() const -> std::optional<FunctionSymbol*> final;
        auto BindDropGlue(FunctionSymbol* const glue) -> void final;
        auto GetDropGlue() const -> std::optional<FunctionSymbol*> final;

        auto CollectVars() const -> std::vector<InstanceVarSymbol*>;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        Ident m_Name{};
        AccessModifier m_AccessModifier{};

        mutable std::optional<InstanceVarSymbol*> m_ResolvingVar{};
        mutable std::optional<DiagnosticBag> m_OptCycleDiagnosticsCache{};
        bool m_IsPrimitivelyEmittable{};
        
        bool m_IsTriviallyCopyable{};
        bool m_IsTriviallyDroppable{};

        std::optional<FunctionSymbol*> m_OptCopyGlue{};
        std::optional<FunctionSymbol*> m_OptDropGlue{};
    };
}
