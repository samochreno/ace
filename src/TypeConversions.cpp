#include "TypeConversions.hpp"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "TypeInfo.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"

namespace Ace
{
    static auto GetNativeConversionOp(
        ITypeSymbol* fromType, 
        ITypeSymbol* toType, 
        const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>& fromOpMap
    ) -> std::optional<FunctionSymbol*>
    {
        const auto fromOpMapIt = fromOpMap.find(toType);
        if (fromOpMapIt == end(fromOpMap))
        {
            return std::nullopt;
        }

        const auto foundOpIt = fromOpMapIt->second.find(fromType);
        if (foundOpIt == end(fromOpMapIt->second))
        {
            return std::nullopt;
        }

        return foundOpIt->second;
    }

    static auto GetImplicitPointerConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* toType
    ) -> Expected<FunctionSymbol*>
    {
        fromType = fromType->GetWithoutReference();
          toType =   toType->GetWithoutReference();

        ACE_TRY_ASSERT(
            fromType->IsStrongPointer() && toType->IsWeakPointer()
        );

        ACE_TRY_ASSERT(
            fromType->GetWithoutStrongPointer() ==
            toType->GetWithoutWeakPointer()
        );

        auto* const compilation = scope->GetCompilation();

        return dynamic_cast<FunctionSymbol*>(Scope::ResolveOrInstantiateTemplateInstance(
            compilation,
            compilation->Natives->WeakPointer__from.GetSymbol(),
            std::nullopt,
            { fromType->GetWithoutStrongPointer()->GetWithoutReference() },
            {}
        ).Unwrap());
    }

    auto GetImplicitConversionOp(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* toType
    ) -> Expected<FunctionSymbol*>
    {
        const auto optNativeOp = GetNativeConversionOp(
            fromType,
            toType,
            scope->GetCompilation()->Natives->GetImplicitFromOpMap()
        );
        if (optNativeOp.has_value())
        {
            return optNativeOp.value();
        }

        return GetImplicitPointerConversionOp(
            scope,
            fromType,
            toType
        );
    }

    auto GetExplicitConversionOp(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* toType
    ) -> Expected<FunctionSymbol*>
    {
        const auto optNativeImplicitOp = GetNativeConversionOp(
            fromType,
            toType,
            scope->GetCompilation()->Natives->GetImplicitFromOpMap()
        );
        if (optNativeImplicitOp.has_value())
        {
            return optNativeImplicitOp.value();
        }

        const auto optNativeExplicitOp = GetNativeConversionOp(
            fromType,
            toType,
            scope->GetCompilation()->Natives->GetExplicitFromOpMap()
        );
        if (optNativeExplicitOp)
        {
            return optNativeExplicitOp.value();
        }

        return GetImplicitPointerConversionOp(
            scope,
            fromType,
            toType
        );
    }

    auto AreTypesSame(
        const std::vector<ITypeSymbol*>& typesA,
        const std::vector<ITypeSymbol*>& typesB
    ) -> bool
    {
        if (typesA.size() != typesB.size())
        {
            return false;
        }

        for (size_t i = 0; i < typesA.size(); i++)
        {
            if (
                typesA.at(i)->GetUnaliased() !=
                typesB.at(i)->GetUnaliased()
                )
            {
                return false;
            }
        }

        return true;
    }

    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& scope,
        const std::vector<TypeInfo>& fromTypeInfos,
        const std::vector<TypeInfo>& targetTypeInfos
    ) -> bool
    {
        if (fromTypeInfos.size() != targetTypeInfos.size())
        {
            return false;
        }

        for (size_t i = 0; i < fromTypeInfos.size(); i++)
        {
            const auto dummyExpr = std::make_shared<const ConversionPlaceholderExprBoundNode>(
                SourceLocation{},
                scope,
                fromTypeInfos.at(i)
            );

            const auto expConvertedExpr = CreateImplicitlyConverted(
                dummyExpr,
                targetTypeInfos.at(i)
            );
            if (expConvertedExpr)
            {
                continue;
            }

            return false;
        }

        return true;
    }
}
