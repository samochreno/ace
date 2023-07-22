#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    auto GetImplicitConversionOp(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> Expected<FunctionSymbol*>;
    auto GetExplicitConversionOp(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> Expected<FunctionSymbol*>;

    auto AreTypesSame(
        const std::vector<ITypeSymbol*>& typesA,
        const std::vector<ITypeSymbol*>& typesB
    ) -> bool;
    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& scope,
        const std::vector<TypeInfo>& fromTypeInfos,
        const std::vector<TypeInfo>& targetTypeInfos
    ) -> bool;
}
