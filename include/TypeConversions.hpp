#pragma once

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    class IExprBoundNode;

    typedef std::optional<FunctionSymbol*>(*ConversionOpGetterFunction)(
        const std::shared_ptr<Scope>&,
        ITypeSymbol*,
        ITypeSymbol*
    );
    auto GetImplicitConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> std::optional<FunctionSymbol*>;
    auto GetExplicitConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> std::optional<FunctionSymbol*>;

    auto AreTypesSame(
        const std::vector<ITypeSymbol*>& typesA,
        const std::vector<ITypeSymbol*>& typesB
    ) -> bool;
    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& scope,
        const std::vector<TypeInfo>& fromTypeInfos,
        const std::vector<TypeInfo>& targetTypeInfos
    ) -> bool;

    auto CreateConverted(
        std::shared_ptr<const IExprBoundNode> expr,
        const TypeInfo& targetTypeInfo,
        const ConversionOpGetterFunction func
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>;
    auto CreateImplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>;
    auto CreateExplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>;

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>;
}
