#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Base.hpp"
#include "Emittable.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Scope.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Expression
{
    class IBase : public virtual BoundNode::IBase, public virtual IEmittable<ExpressionEmitResult>
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> = 0;
        virtual auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> = 0;

        virtual auto GetTypeInfo() const -> TypeInfo = 0;
    };

    typedef Expected<Symbol::Function*>(*ConversionOperatorGetterFunction)(const std::shared_ptr<Scope>&, Symbol::Type::IBase*, Symbol::Type::IBase*);
    auto CreateConverted(std::shared_ptr<const BoundNode::Expression::IBase> t_expression, TypeInfo t_targetTypeInfo, ConversionOperatorGetterFunction t_func) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>;
    inline auto CreateImplicitlyConverted(const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression, const TypeInfo& t_targetTypeInfo)
    {
        return CreateConverted(t_expression, t_targetTypeInfo, &Symbol::Type::GetImplicitConversionOperator);
    }
    inline auto CreateExplicitlyConverted(const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression, const TypeInfo& t_targetTypeInfo)
    {
        return CreateConverted(t_expression, t_targetTypeInfo, &Symbol::Type::GetExplicitConversionOperator);
    }

    typedef Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>(*ConversionFunction)(const std::shared_ptr<const BoundNode::Expression::IBase>&, const TypeInfo&);
    auto CreateConvertedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const TypeInfo& t_targetTypeInfo, ConversionFunction t_func) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>;
    auto CreateConvertedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const std::vector<TypeInfo>& t_targetTypeInfos, ConversionFunction t_func) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>;
    template<typename T>
    auto CreateImplicitlyConvertedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const T& t_targetTypeInfo) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>
    {
        return CreateConvertedVector(t_expressions, t_targetTypeInfo, &CreateImplicitlyConverted);
    }
    template<typename T>
    auto CreateExplicitlyConvertedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const T& t_targetTypeInfo) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>
    {
        return CreateConvertedVector(t_expressions, t_targetTypeInfo, &CreateExplicitlyConverted);
    }

    auto CreateImplicitlyConvertedAndTypeChecked(const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression, const TypeInfo& t_targetTypeInfo) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>;
    template<typename T>
    auto CreateImplicitlyConvertedAndTypeCheckedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const T& t_targetTypeInfo) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConvertedVector(t_expressions, t_targetTypeInfo));

        ACE_TRY(mchChecked, TransformExpectedMaybeChangedVector(mchConverted.Value,
        [](const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)
        {
            return t_expression->GetOrCreateTypeCheckedExpression({});
        }));

        if (!mchConverted.IsChanged && !mchChecked.IsChanged)
            return CreateUnchanged(t_expressions);

        return CreateChanged(mchChecked.Value);
    }
    auto CreateImplicitlyConvertedAndTypeCheckedOptional(const std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>& t_optExpression, const TypeInfo& t_targetTypeInfo) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>>>;
}
