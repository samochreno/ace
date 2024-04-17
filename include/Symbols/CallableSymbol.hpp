#pragma once

#include <vector>
#include <optional>

#include "Symbols/TypedSymbol.hpp"
#include "Symbols/BodyScopedSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/BodyScopedSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class ICallableSymbol :
        public virtual ITypedSymbol,
        public virtual IBodyScopedSymbol,
        public virtual IGenericSymbol
    {
    public:
        virtual ~ICallableSymbol() = default;

        virtual auto CollectParams()    const -> std::vector<IParamVarSymbol*> final;
        virtual auto CollectAllParams() const -> std::vector<IParamVarSymbol*> final;
        virtual auto CollectSelfParam() const -> std::optional<SelfParamVarSymbol*> final;

        virtual auto CollectParamTypes() const -> std::vector<ITypeSymbol*> final;

        virtual auto CollectArgTypeInfos()    const -> std::vector<TypeInfo> final;
        virtual auto CollectAllArgTypeInfos() const -> std::vector<TypeInfo> final;

        virtual auto CollectSelfType() const -> std::optional<ITypeSymbol*> final;
    };
}
