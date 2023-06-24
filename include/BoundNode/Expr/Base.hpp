#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Base.hpp"
#include "Emittable.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Scope.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Asserts.hpp"
#include "TypeConversions.hpp"

namespace Ace::BoundNode::Expr
{
    class IBase :
        public virtual BoundNode::IBase,
        public virtual IEmittable<ExprEmitResult>
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetOrCreateTypeCheckedExpr(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> = 0;
        virtual auto GetOrCreateLoweredExpr(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> = 0;

        virtual auto GetTypeInfo() const -> TypeInfo = 0;
    };

    typedef Expected<Symbol::Function*>(*ConversionOperatorGetterFunction)(const std::shared_ptr<Scope>&, Symbol::Type::IBase*, Symbol::Type::IBase*);
    auto CreateConverted(
        std::shared_ptr<const BoundNode::Expr::IBase> t_expr,
        TypeInfo t_targetTypeInfo,
        ConversionOperatorGetterFunction t_func
    ) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>;
    inline auto CreateImplicitlyConverted(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        const TypeInfo& t_targetTypeInfo
    )
    {
        return CreateConverted(
            t_expr,
            t_targetTypeInfo,
            &GetImplicitConversionOperator
        );
    }
    inline auto CreateExplicitlyConverted(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        const TypeInfo& t_targetTypeInfo
    )
    {
        return CreateConverted(
            t_expr,
            t_targetTypeInfo,
            &GetExplicitConversionOperator
        );
    }

    typedef Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>(*ConversionFunction)(const std::shared_ptr<const BoundNode::Expr::IBase>&, const TypeInfo&);
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const TypeInfo& t_targetTypeInfo,
        ConversionFunction t_func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>;
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const std::vector<TypeInfo>& t_targetTypeInfos,
        ConversionFunction t_func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>;
    template<typename T>
    auto CreateImplicitlyConvertedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const T& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>
    {
        return CreateConvertedVector(
            t_exprs,
            t_targetTypeInfo,
            &CreateImplicitlyConverted
        );
    }
    template<typename T>
    auto CreateExplicitlyConvertedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const T& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>
    {
        return CreateConvertedVector(
            t_exprs,
            t_targetTypeInfo,
            &CreateExplicitlyConverted
        );
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        const TypeInfo& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>;
    template<typename T>
    auto CreateImplicitlyConvertedAndTypeCheckedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const T& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConvertedVector(
            t_exprs,
            t_targetTypeInfo
        ));

        ACE_TRY(mchChecked, TransformExpectedMaybeChangedVector(mchConverted.Value,
        [](const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr)
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
        const std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>& t_optExpr,
        const TypeInfo& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>>>;
}
