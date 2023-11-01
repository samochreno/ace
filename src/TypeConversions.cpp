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
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/ConversionPlaceholderExprSema.hpp"
#include "Semas/Exprs/RefExprSema.hpp"
#include "Semas/Exprs/DerefExprSema.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"

namespace Ace
{
    static auto GetNativeConversionOp(
        ITypeSymbol* fromType, 
        ITypeSymbol* targetType, 
        const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>& fromOpMap
    ) -> std::optional<FunctionSymbol*>
    {
        const auto fromOpMapIt = fromOpMap.find(targetType);
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

    static auto GetStrongPtrToWeakPtrConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        fromType   =   fromType->GetWithoutRef();
        targetType = targetType->GetWithoutRef();

        if (!fromType->IsAnyStrongPtr() || !targetType->IsWeakPtr())
        {
            return std::nullopt;
        }

        auto* const pureFromType = fromType->GetWithoutStrongPtr();
        auto* const pureTargetType = targetType->GetWithoutWeakPtr();

        if (pureFromType != pureTargetType)
        {
            return std::nullopt;
        }

        auto* const compilation = scope->GetCompilation();

        const auto isSized =
            dynamic_cast<ISizedTypeSymbol*>(pureFromType->GetUnaliased()) !=
            nullptr;

        auto* const functionRoot = isSized ?
            compilation->GetNatives().weak_ptr_from.GetSymbol() :
            compilation->GetNatives().weak_ptr_from_dyn.GetSymbol();

        auto* const function = dynamic_cast<FunctionSymbol*>(
            Scope::ForceCollectGenericInstance(functionRoot, { pureFromType })
        );
        ACE_ASSERT(function);
        return function;
    }

    static auto GetStrongPtrToDynStrongPtrConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        fromType   =   fromType->GetWithoutRef();
        targetType = targetType->GetWithoutRef();

        if (!fromType->IsAnyStrongPtr() || !targetType->IsAnyStrongPtr())
        {
            return std::nullopt;
        }

        auto* const pureFromType = fromType->GetWithoutStrongPtr();
        auto* const pureTargetType = targetType->GetWithoutStrongPtr();

        auto* concreteType =
            dynamic_cast<ISizedTypeSymbol*>(pureFromType->GetUnaliased());
        if (!concreteType)
        {
            return std::nullopt;
        }

        auto* const targetTrait =
            dynamic_cast<TraitTypeSymbol*>(pureTargetType->GetUnaliased());

        if (!targetTrait)
        {
            return std::nullopt;
        }

        const auto optImplSymbol = Scope::CollectImplOfFor(
            targetTrait,
            concreteType
        );
        if (!optImplSymbol.has_value())
        {
            return std::nullopt;
        }

        auto* const compilation = scope->GetCompilation();

        auto* const function = Scope::ForceCollectGenericInstance(
            compilation->GetNatives().strong_ptr_to_dyn_strong_ptr.GetSymbol(),
            { pureFromType, pureTargetType }
        );
        return dynamic_cast<FunctionSymbol*>(function);
    }

    static auto GetImplicitPtrConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const fromType,
        ITypeSymbol* const targetType
    ) -> std::optional<FunctionSymbol*>
    {
        std::optional<FunctionSymbol*> optConversionOp{};

        optConversionOp =
            GetStrongPtrToWeakPtrConversionOp(scope, fromType, targetType);
        if (optConversionOp.has_value())
        {
            return optConversionOp.value();
        }

        optConversionOp =
            GetStrongPtrToDynStrongPtrConversionOp(scope, fromType, targetType);
        if (optConversionOp.has_value())
        {
            return optConversionOp.value();
        }

        return std::nullopt;
    }

    auto GetImplicitConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> std::optional<FunctionSymbol*>
    {
        const auto optNativeOp = GetNativeConversionOp(
            fromType,
            targetType,
            scope->GetCompilation()->GetNatives().GetImplicitFromOpMap()
        );
        if (optNativeOp.has_value())
        {
            return optNativeOp.value();
        }

        return GetImplicitPtrConversionOp(scope, fromType, targetType);
    }

    auto GetExplicitConversionOp(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* fromType,
        ITypeSymbol* targetType
    ) -> std::optional<FunctionSymbol*>
    {
        const auto optNativeImplicitOp = GetNativeConversionOp(
            fromType,
            targetType,
            scope->GetCompilation()->GetNatives().GetImplicitFromOpMap()
        );
        if (optNativeImplicitOp.has_value())
        {
            return optNativeImplicitOp.value();
        }

        const auto optNativeExplicitOp = GetNativeConversionOp(
            fromType,
            targetType,
            scope->GetCompilation()->GetNatives().GetExplicitFromOpMap()
        );
        if (optNativeExplicitOp)
        {
            return optNativeExplicitOp.value();
        }

        return GetImplicitPtrConversionOp(scope, fromType, targetType);
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
            const auto placeholderExpr = std::make_shared<const ConversionPlaceholderExprSema>(
                SrcLocation{ scope->GetCompilation() },
                scope,
                fromTypeInfos.at(i)
            );

            const auto convertedExpr = CreateImplicitlyConverted(
                placeholderExpr,
                targetTypeInfos.at(i)
            );
            if (convertedExpr.GetDiagnostics().HasErrors())
            {
                return false;
            }
        }

        return true;
    }

    static auto CreateConvertedReturn(
        const std::shared_ptr<const IExprSema>& expr,
        DiagnosticBag diagnostics
    ) -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        const bool isAlreadyError = expr->GetTypeInfo().Symbol->IsError();
        if (!diagnostics.HasErrors() && !isAlreadyError)
        {
            return Diagnosed{ expr, std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const ConversionPlaceholderExprSema>(
                expr->GetSrcLocation(),
                expr->GetScope(),
                TypeInfo
                {
                    expr->GetCompilation()->GetErrorSymbols().GetType(),
                    ValueKind::R,
                }
            ),
            std::move(diagnostics),
        };
    }

    auto CreateConverted(
        std::shared_ptr<const IExprSema> expr,
        const TypeInfo& targetTypeInfo,
        const ConversionOpGetterFunction function
    ) -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (
            targetTypeInfo.Symbol->IsError() ||
            expr->GetTypeInfo().Symbol->IsError()
            )
        {
            return CreateConvertedReturn(expr, std::move(diagnostics));
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
            return CreateConvertedReturn(expr, std::move(diagnostics));
        }

        const bool isRef = expr->GetTypeInfo().Symbol->IsRef();
        const bool isTargetRef = targetTypeInfo.Symbol->IsRef();

        if (isRef)
        {
            if (!isTargetRef)
            {
                expr = std::make_shared<const DerefExprSema>(
                    expr->GetSrcLocation(),
                    expr
                );
            }
        }
        else
        {
            if (isTargetRef)
            {
                expr = std::make_shared<const RefExprSema>(
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
            return CreateConvertedReturn(expr, std::move(diagnostics));
        }

        const auto optOpSymbol = function(
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
            return CreateConvertedReturn(expr, std::move(diagnostics));
        }

        return CreateConvertedReturn(
            std::make_shared<const StaticCallExprSema>(
                expr->GetSrcLocation(),
                expr->GetScope(),
                optOpSymbol.value(),
                std::vector{ expr }
            ),
            std::move(diagnostics)
        );
    }

    auto CreateImplicitlyConverted(
        const std::shared_ptr<const IExprSema>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateConverted(expr, targetTypeInfo, &GetImplicitConversionOp);
    }

    auto CreateExplicitlyConverted(
        const std::shared_ptr<const IExprSema>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateConverted(expr, targetTypeInfo, &GetExplicitConversionOp);
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const IExprSema>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto convertedExpr = diagnostics.Collect(
            CreateImplicitlyConverted(expr, targetTypeInfo)
        );

        const auto checkedExpr = diagnostics.Collect(
            convertedExpr->CreateTypeCheckedExpr({})
        );

        return Diagnosed{ checkedExpr, std::move(diagnostics) };
    }
}
