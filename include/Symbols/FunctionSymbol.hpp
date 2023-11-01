#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Symbols/Symbol.hpp"
#include "Symbols/TypedSymbol.hpp"
#include "Symbols/BodyScopedSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Symbols/ConstrainedSymbol.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "TypeInfo.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class BlockStmtSema;

    class FunctionSymbol :
        public virtual ISymbol,
        public virtual ITypedSymbol,
        public virtual IBodyScopedSymbol,
        public virtual IGenericSymbol,
        public virtual IConstrainedSymbol,
        public virtual ICallableSymbol
    {
    public:
        FunctionSymbol(
            const std::shared_ptr<Scope>& bodyScope,
            const SymbolCategory category,
            const AccessModifier accessModifier,
            const Ident& name,
            ITypeSymbol* const type,
            const std::vector<ITypeSymbol*>& typeArgs
        );
        virtual ~FunctionSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetType() const -> ITypeSymbol* final;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;

        auto GetConstrainedScope() const -> std::shared_ptr<Scope> final;

        auto BindBlockSema(
            const std::shared_ptr<const BlockStmtSema>& blockSema
        ) -> void;
        auto GetBlockSema() -> const std::optional<std::shared_ptr<const BlockStmtSema>>&;

        auto BindEmittableBlock(
            const std::shared_ptr<const IEmittable<void>>& emittableBlock
        ) -> void;
        auto GetEmittableBlock() const -> const std::optional<std::shared_ptr<const IEmittable<void>>>&;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        SymbolCategory m_Category{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        ITypeSymbol* m_Type{};
        std::vector<ITypeSymbol*> m_TypeArgs{};

        std::optional<std::shared_ptr<const BlockStmtSema>> m_OptBlockSema{};
        std::optional<std::shared_ptr<const IEmittable<void>>> m_OptEmittableBlock{};
    };
}
