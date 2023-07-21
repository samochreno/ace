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
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> = 0;
        virtual auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
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
        std::shared_ptr<const IExprBoundNode> t_expr,
        TypeInfo t_targetTypeInfo,
        ConversionOpGetterFunction t_func
    ) -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>;
    inline auto CreateImplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        const TypeInfo& t_targetTypeInfo
    )
    {
        return CreateConverted(
            t_expr,
            t_targetTypeInfo,
            &GetImplicitConversionOp
        );
    }
    inline auto CreateExplicitlyConverted(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        const TypeInfo& t_targetTypeInfo
    )
    {
        return CreateConverted(
            t_expr,
            t_targetTypeInfo,
            &GetExplicitConversionOp
        );
    }

    typedef Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>(*ConversionFunction)(const std::shared_ptr<const IExprBoundNode>&, const TypeInfo&);
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_exprs,
        const TypeInfo& t_targetTypeInfo,
        ConversionFunction t_func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>;
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_exprs,
        const std::vector<TypeInfo>& t_targetTypeInfos,
        ConversionFunction t_func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>;
    template<typename T>
    auto CreateImplicitlyConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_exprs,
        const T& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        return CreateConvertedVector(
            t_exprs,
            t_targetTypeInfo,
            &CreateImplicitlyConverted
        );
    }
    template<typename T>
    auto CreateExplicitlyConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_exprs,
        const T& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        return CreateConvertedVector(
            t_exprs,
            t_targetTypeInfo,
            &CreateExplicitlyConverted
        );
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        const TypeInfo& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>;
    template<typename T>
    auto CreateImplicitlyConvertedAndTypeCheckedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_exprs,
        const T& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConvertedVector(
            t_exprs,
            t_targetTypeInfo
        ));

        ACE_TRY(mchChecked, TransformExpectedMaybeChangedVector(mchConverted.Value,
        [](const std::shared_ptr<const IExprBoundNode>& t_expr)
        {
            return t_expr->GetOrCreateTypeCheckedExpr({});
        }));

        if (
            !mchConverted.IsChanged &&
            !mchChecked.IsChanged
            )
        {
            return CreateUnchanged(t_exprs);
        }

        return CreateChanged(mchChecked.Value);
    }
    auto CreateImplicitlyConvertedAndTypeCheckedOptional(
        const std::optional<std::shared_ptr<const IExprBoundNode>>& t_optExpr,
        const TypeInfo& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const IExprBoundNode>>>>;
}
