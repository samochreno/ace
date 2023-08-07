#include "TypeConversions.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <unordered_map>

#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"
#include "BoundNodes/Exprs/RefExprBoundNode.hpp"
#include "BoundNodes/Exprs/DerefExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"

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

    static auto GetImplicitPtrConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* toType
    ) -> std::optional<FunctionSymbol*>
    {
        fromType = fromType->GetWithoutRef();
          toType =   toType->GetWithoutRef();

        if (!fromType->IsStrongPtr() || !toType->IsWeakPtr())
        {
            return std::nullopt;
        }

        if (fromType->GetWithoutStrongPtr() != toType->GetWithoutWeakPtr())
        {
            return std::nullopt;
        }

        auto* const compilation = scope->GetCompilation();

        auto* const function = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            compilation->GetNatives()->WeakPtr__from.GetSymbol(),
            std::nullopt,
            { fromType->GetWithoutStrongPtr()->GetWithoutRef() },
            {}
        ).Unwrap();
        return dynamic_cast<FunctionSymbol*>(function);
    }

    auto GetImplicitConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* toType
    ) -> std::optional<FunctionSymbol*>
    {
        const auto optNativeOp = GetNativeConversionOp(
            fromType,
            toType,
            scope->GetCompilation()->GetNatives()->GetImplicitFromOpMap()
        );
        if (optNativeOp.has_value())
        {
            return optNativeOp.value();
        }

        return GetImplicitPtrConversionOp(
            scope,
            fromType,
            toType
        );
    }

    auto GetExplicitConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* toType
    ) -> std::optional<FunctionSymbol*>
    {
        const auto optNativeImplicitOp = GetNativeConversionOp(
            fromType,
            toType,
            scope->GetCompilation()->GetNatives()->GetImplicitFromOpMap()
        );
        if (optNativeImplicitOp.has_value())
        {
            return optNativeImplicitOp.value();
        }

        const auto optNativeExplicitOp = GetNativeConversionOp(
            fromType,
            toType,
            scope->GetCompilation()->GetNatives()->GetExplicitFromOpMap()
        );
        if (optNativeExplicitOp)
        {
            return optNativeExplicitOp.value();
        }

        return GetImplicitPtrConversionOp(
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
            if (typesA.at(i)->GetUnaliased() != typesB.at(i)->GetUnaliased())
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
            const auto placeholderExpr = std::make_shared<const ConversionPlaceholderExprBoundNode>(
                SrcLocation{},
                scope,
                fromTypeInfos.at(i)
            );

            const auto dgnConvertedExpr = CreateImplicitlyConverted(
                placeholderExpr,
                targetTypeInfos.at(i)
            );
            if (dgnConvertedExpr.GetDiagnostics().HasErrors())
            {
                return false;
            }
        }

        return true;
    }

    auto CreateConverted(
        std::shared_ptr<const IExprBoundNode> expr,
        const TypeInfo& targetTypeInfo,
        const ConversionOpGetterFunction func
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        if (
            targetTypeInfo.Symbol->IsError() ||
            expr->GetTypeInfo().Symbol->IsError()
            )
        {
            return Diagnosed{ expr, diagnostics };
        }

        if (
            (targetTypeInfo.ValueKind == ValueKind::L) &&
            (expr->GetTypeInfo().ValueKind == ValueKind::R)
            )
        {
            diagnostics.Add(CreateExpectedLValueExprError(expr));
        }

        if (
            expr->GetTypeInfo().Symbol->GetUnaliased() ==
            targetTypeInfo.Symbol->GetUnaliased()
            )
        {
            return Diagnosed{ expr, diagnostics };
        }

        const bool isRef = expr->GetTypeInfo().Symbol->IsRef();
        const bool isTargetRef = targetTypeInfo.Symbol->IsRef();

        if (isRef)
        {
            if (!isTargetRef)
            {
                expr = std::make_shared<const DerefExprBoundNode>(
                    expr->GetSrcLocation(),
                    expr
                );
            }
        }
        else
        {
            if (isTargetRef)
            {
                expr = std::make_shared<const RefExprBoundNode>(
                    expr->GetSrcLocation(),
                    expr
                );
            }
        }

        if (
            expr->GetTypeInfo().Symbol->GetUnaliased() ==
            targetTypeInfo.Symbol->GetUnaliased()
            )
        {
            return Diagnosed{ expr, diagnostics };
        }

        const auto optOpSymbol = func(
            expr->GetScope(),
            expr->GetTypeInfo().Symbol,
            targetTypeInfo.Symbol
        );
        if (!optOpSymbol.has_value())
        {
            diagnostics.Add(CreateUnableToConvertExprError(
                expr,
                targetTypeInfo
            ));
            return Diagnosed{ expr, diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const StaticFunctionCallExprBoundNode>(
                expr->GetSrcLocation(),
                expr->GetScope(),
                optOpSymbol.value(),
                std::vector{ expr }
            ),
            diagnostics,
        };
    }

    auto CreateImplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateConverted(
            expr,
            targetTypeInfo,
            &GetImplicitConversionOp
        );
    }

    auto CreateExplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateConverted(
            expr,
            targetTypeInfo,
            &GetExplicitConversionOp
        );
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto convertedExpr = diagnostics.Collect(CreateImplicitlyConverted(
            expr,
            targetTypeInfo
        ));

        const auto checkedExpr =
            diagnostics.Collect(convertedExpr->CreateTypeCheckedExpr({}));

        return Diagnosed{ checkedExpr, diagnostics };
    }
}
