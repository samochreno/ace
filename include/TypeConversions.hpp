#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    auto GetImplicitConversionOperator(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* t_fromType,
        ITypeSymbol* t_targetType
    ) -> Expected<FunctionSymbol*>;
    auto GetExplicitConversionOperator(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* t_fromType,
        ITypeSymbol* t_targetType
    ) -> Expected<FunctionSymbol*>;

    auto AreTypesSame(
        const std::vector<ITypeSymbol*>& t_typesA,
        const std::vector<ITypeSymbol*>& t_typesB
    ) -> bool;
    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<TypeInfo>& t_fromTypeInfos,
        const std::vector<TypeInfo>& t_targetTypeInfos
    ) -> bool;
}
