#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class NormalTemplateParamTypeSymbol : public virtual ITypeSymbol
    {
    public:
        NormalTemplateParamTypeSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name
        );
        virtual ~NormalTemplateParamTypeSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectTemplateArgs() const -> std::vector<ITypeSymbol*> final;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*> final;

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

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
    };
}

