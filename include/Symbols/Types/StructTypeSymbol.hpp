#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class StructTypeSymbol : public virtual ITypeSymbol
    {
    public:
        StructTypeSymbol(
            const std::shared_ptr<Scope>& selfScope,
            const Identifier& name,
            const AccessModifier accessModifier
        );
        virtual ~StructTypeSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetSizeKind() const -> Expected<TypeSizeKind> final;
        auto SetAsUnsized() -> void final;
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

        auto CollectTemplateArgs() const -> std::vector<ITypeSymbol*> final;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*> final;

        auto GetVars() const -> std::vector<InstanceVarSymbol*>;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        Identifier m_Name{};
        AccessModifier m_AccessModifier{};

        mutable bool m_IsResolvingSize{};
        mutable std::optional<Expected<TypeSizeKind>> m_OptSizeKindCache{};
        bool m_IsUnsized{};
        bool m_IsPrimitivelyEmittable{};
        
        bool m_IsTriviallyCopyable{};
        bool m_IsTriviallyDroppable{};

        std::optional<FunctionSymbol*> m_OptCopyGlue{};
        std::optional<FunctionSymbol*> m_OptDropGlue{};
    };
}
