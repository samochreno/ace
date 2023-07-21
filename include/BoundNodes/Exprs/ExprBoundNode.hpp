#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/BoundNode.hpp"
#include "Emittable.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "SourceLocation.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Assert.hpp"
#include "TypeConversions.hpp"

namespace Ace
{
    class IExprBoundNode :
        public virtual IBoundNode,
        public virtual IEmittable<ExprEmitResult>
    {
    public:
        virtual ~IExprBoundNode() = default;

        virtual auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> = 0;
        virtual auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> = 0;

        virtual auto GetTypeInfo() const -> TypeInfo = 0;
    };

    typedef Expected<FunctionSymbol*>(*ConversionOpGetterFunction)(
        const SourceLocation&,
        const std::shared_ptr<Scope>&,
        ITypeSymbol*,
        ITypeSymbol*
    );
    auto CreateConverted(
        std::shared_ptr<const IExprBoundNode> expr,
        TypeInfo targetTypeInfo,
        ConversionOpGetterFunction func
    ) -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>;
    inline auto CreateImplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    )
    {
        return CreateConverted(
            expr,
            targetTypeInfo,
            &GetImplicitConversionOp
        );
    }
    inline auto CreateExplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    )
    {
        return CreateConverted(
            expr,
            targetTypeInfo,
            &GetExplicitConversionOp
        );
    }

    typedef Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>(*ConversionFunction)(const std::shared_ptr<const IExprBoundNode>&, const TypeInfo&);
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const TypeInfo& targetTypeInfo,
        ConversionFunction func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>;
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const std::vector<TypeInfo>& targetTypeInfos,
        ConversionFunction func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>;
    template<typename T>
    auto CreateImplicitlyConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const T& targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        return CreateConvertedVector(
            exprs,
            targetTypeInfo,
            &CreateImplicitlyConverted
        );
    }
    template<typename T>
    auto CreateExplicitlyConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const T& targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        return CreateConvertedVector(
            exprs,
            targetTypeInfo,
            &CreateExplicitlyConverted
        );
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>;
    template<typename T>
    auto CreateImplicitlyConvertedAndTypeCheckedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const T& targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConvertedVector(
            exprs,
            targetTypeInfo
        ));

        ACE_TRY(mchChecked, TransformExpectedMaybeChangedVector(mchConverted.Value,
        [](const std::shared_ptr<const IExprBoundNode>& expr)
        {
            return expr->GetOrCreateTypeCheckedExpr({});
        }));

        if (
            !mchConverted.IsChanged &&
            !mchChecked.IsChanged
            )
        {
            return CreateUnchanged(exprs);
        }

        return CreateChanged(mchChecked.Value);
    }
    auto CreateImplicitlyConvertedAndTypeCheckedOptional(
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr,
        const TypeInfo& targetTypeInfo
    ) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const IExprBoundNode>>>>;
}
