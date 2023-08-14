#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Symbols/ParamizedSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "TypeInfo.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class FunctionTemplateSymbol;

    class FunctionSymbol :
        public virtual ISymbol,
        public virtual ISelfScopedSymbol,
        public virtual ITemplatableSymbol,
        public virtual IParamizedSymbol
    {
    public:
        FunctionSymbol(
            const std::shared_ptr<Scope>& selfScope,
            const Ident& name,
            const SymbolCategory symbolCategory,
            const AccessModifier accessModifier,
            ITypeSymbol* const type
        );
        virtual ~FunctionSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectParams()    const -> std::vector<IParamVarSymbol*> final;
        auto CollectAllParams() const -> std::vector<IParamVarSymbol*>;
        auto CollectSelfParam() const -> std::optional<SelfParamVarSymbol*>;

        auto CollectArgTypeInfos() const -> std::vector<TypeInfo>;

        auto GetType() const -> ITypeSymbol*;

        auto BindBody(const std::shared_ptr<const IEmittable<void>>& body) -> void;
        auto GetBody() -> std::optional<const IEmittable<void>*>;

        auto GetTemplate() const -> std::optional<FunctionTemplateSymbol*>;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        Ident m_Name{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        ITypeSymbol* m_Type{};

        std::optional<std::shared_ptr<const IEmittable<void>>> m_OptBody{};
    };
}
