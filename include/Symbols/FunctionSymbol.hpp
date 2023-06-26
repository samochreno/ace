#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>

#include "Symbols/Symbol.hpp"
#include "Symbols/TypedSymbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Symbols/ParamizedSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "TypeInfo.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class FunctionTemplateSymbol;

    class FunctionSymbol :
        public virtual ISymbol,
        public virtual ITypedSymbol,
        public virtual ISelfScopedSymbol,
        public virtual ITemplatableSymbol,
        public virtual IParamizedSymbol
    {
    public:
        FunctionSymbol(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const SymbolCategory& t_symbolCategory,
            const AccessModifier& t_accessModifier,
            ITypeSymbol* const t_type
        );
        virtual ~FunctionSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

        auto CollectParams()    const -> std::vector<IParamVarSymbol*> final;
        auto CollectAllParams() const -> std::vector<IParamVarSymbol*>;

        auto CollectArgTypeInfos() const -> std::vector<TypeInfo>;

        auto BindBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void;
        auto GetBody() -> std::optional<const IEmittable<void>*>;

        auto GetTemplate() const -> std::optional<FunctionTemplateSymbol*>;

        auto CollectTemplateArgs()     const -> std::vector<ITypeSymbol*> final;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*> final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        ITypeSymbol* m_Type{};

        std::optional<std::shared_ptr<const IEmittable<void>>> m_OptBody{};
    };
}
