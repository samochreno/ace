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
        ITypeSymbol* t_fromType, 
        ITypeSymbol* t_toType, 
        const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>& t_fromOpMap
    ) -> std::optional<FunctionSymbol*>
    {
        const auto fromOpMapIt = t_fromOpMap.find(t_toType);
        if (fromOpMapIt == end(t_fromOpMap))
        {
            return std::nullopt;
        }

        const auto foundOpIt = fromOpMapIt->second.find(t_fromType);
        if (foundOpIt == end(fromOpMapIt->second))
        {
            return std::nullopt;
        }

        return foundOpIt->second;
    }

    static auto GetImplicitPointerConversionOp(
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* t_fromType,
        ITypeSymbol* t_toType
    ) -> Expected<FunctionSymbol*>
    {
        t_fromType = t_fromType->GetWithoutReference();
          t_toType =   t_toType->GetWithoutReference();

        ACE_TRY_ASSERT(
            t_fromType->IsStrongPointer() && t_toType->IsWeakPointer()
        );

        ACE_TRY_ASSERT(
            t_fromType->GetWithoutStrongPointer() ==
            t_toType->GetWithoutWeakPointer()
        );

        auto* const compilation = t_scope->GetCompilation();

        return dynamic_cast<FunctionSymbol*>(Scope::ResolveOrInstantiateTemplateInstance(
            compilation,
            compilation->Natives->WeakPointer__from.GetSymbol(),
            std::nullopt,
            { t_fromType->GetWithoutStrongPointer()->GetWithoutReference() },
            {}
        ).Unwrap());
    }

    auto GetImplicitConversionOp(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* t_fromType,
        ITypeSymbol* t_toType
    ) -> Expected<FunctionSymbol*>
    {
        const auto optNativeOp = GetNativeConversionOp(
            t_fromType,
            t_toType,
            t_scope->GetCompilation()->Natives->GetImplicitFromOpMap()
        );
        if (optNativeOp.has_value())
        {
            return optNativeOp.value();
        }

        return GetImplicitPointerConversionOp(
            t_scope,
            t_fromType,
            t_toType
        );
    }

    auto GetExplicitConversionOp(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* t_fromType,
        ITypeSymbol* t_toType
    ) -> Expected<FunctionSymbol*>
    {
        const auto optNativeImplicitOp = GetNativeConversionOp(
            t_fromType,
            t_toType,
            t_scope->GetCompilation()->Natives->GetImplicitFromOpMap()
        );
        if (optNativeImplicitOp.has_value())
        {
            return optNativeImplicitOp.value();
        }

        const auto optNativeExplicitOp = GetNativeConversionOp(
            t_fromType,
            t_toType,
            t_scope->GetCompilation()->Natives->GetExplicitFromOpMap()
        );
        if (optNativeExplicitOp)
        {
            return optNativeExplicitOp.value();
        }

        return GetImplicitPointerConversionOp(
            t_scope,
            t_fromType,
            t_toType
        );
    }

    auto AreTypesSame(
        const std::vector<ITypeSymbol*>& t_typesA,
        const std::vector<ITypeSymbol*>& t_typesB
    ) -> bool
    {
        if (t_typesA.size() != t_typesB.size())
        {
            return false;
        }

        for (size_t i = 0; i < t_typesA.size(); i++)
        {
            if (
                t_typesA.at(i)->GetUnaliased() !=
                t_typesB.at(i)->GetUnaliased()
                )
            {
                return false;
            }
        }

        return true;
    }

    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<TypeInfo>& t_fromTypeInfos,
        const std::vector<TypeInfo>& t_targetTypeInfos
    ) -> bool
    {
        if (t_fromTypeInfos.size() != t_targetTypeInfos.size())
        {
            return false;
        }

        for (size_t i = 0; i < t_fromTypeInfos.size(); i++)
        {
            const auto dummyExpr = std::make_shared<const ConversionPlaceholderExprBoundNode>(
                SourceLocation{},
                t_scope,
                t_fromTypeInfos.at(i)
            );

            const auto expConvertedExpr = CreateImplicitlyConverted(
                dummyExpr,
                t_targetTypeInfos.at(i)
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
