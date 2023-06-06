#include "BoundNode/Expression/Base.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <iterator>

#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "BoundNode/Expression//Reference.hpp"
#include "BoundNode/Expression//Dereference.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Expression
{
    auto CreateConvertedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const TypeInfo& t_targetTypeInfo, ConversionFunction t_func) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>
    {
        bool isChanged = false;
        std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> convertedExpressions{};
        convertedExpressions.reserve(t_expressions.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(t_expressions), end(t_expressions),
        [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)
        {
            auto expMchConvertedExpression = t_func(t_expression, t_targetTypeInfo);
            if (!expMchConvertedExpression)
                return false;

            if (expMchConvertedExpression.Unwrap().IsChanged)
                isChanged = true;

            convertedExpressions.push_back(expMchConvertedExpression.Unwrap().Value);
            return true;

        }) == end(t_expressions));

        if (!isChanged)
            return CreateUnchanged(t_expressions);

        return CreateChanged(convertedExpressions);
    }

    auto CreateConvertedVector(const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_expressions, const std::vector<TypeInfo>& t_targetTypeInfos, ConversionFunction t_func) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>>>
    {
        bool isChanged = false;
        std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> convertedExpressions{};
        convertedExpressions.reserve(t_expressions.size());

        for (size_t i = 0; i < t_expressions.size(); i++)
        {
            ACE_TRY(mchConvertedExpression, t_func(t_expressions[i], t_targetTypeInfos[i]));

            if (mchConvertedExpression.IsChanged)
                isChanged = true;

            convertedExpressions.push_back(mchConvertedExpression.Value);
        }

        if (!isChanged)
            return CreateUnchanged(t_expressions);

        return CreateChanged(convertedExpressions);
    }

    auto CreateConverted(std::shared_ptr<const BoundNode::Expression::IBase> t_expression, TypeInfo t_targetTypeInfo, ConversionOperatorGetterFunction t_func) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        if (t_targetTypeInfo.ValueKind == ValueKind::L)
        {
            ACE_TRY_ASSERT(t_expression->GetTypeInfo().ValueKind != ValueKind::R);
        }

        if (t_expression->GetTypeInfo().Symbol->GetUnaliased() == t_targetTypeInfo.Symbol->GetUnaliased())
        {
            return CreateUnchanged(t_expression);
        }

        const bool isReference = t_expression->GetTypeInfo().Symbol->IsReference();
        const bool isTargetReference = t_targetTypeInfo.Symbol->IsReference();

        if (isReference)
        {
            if (!isTargetReference)
            {
                t_expression = std::make_shared<const BoundNode::Expression::Dereference>(t_expression);
            }
        }
        else
        {
            if (isTargetReference)
            {
                t_expression = std::make_shared<const BoundNode::Expression::Reference>(t_expression);
            }
        }

        if (t_expression->GetTypeInfo().Symbol->GetUnaliased() == t_targetTypeInfo.Symbol->GetUnaliased())
        {
            return CreateChanged(t_expression);
        }

        ACE_TRY(operatorSymbol, t_func(
            t_expression->GetScope(), 
            t_expression->GetTypeInfo().Symbol,
            t_targetTypeInfo.Symbol
        ));

        t_expression = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            t_expression->GetScope(),
            operatorSymbol,
            std::vector{ t_expression }
        );

        return CreateChanged(t_expression);
    }

    auto CreateImplicitlyConvertedAndTypeChecked(const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression, const TypeInfo& t_targetTypeInfo) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConverted(t_expression, t_targetTypeInfo));

        ACE_TRY(mchChecked, mchConverted.Value->GetOrCreateTypeCheckedExpression({}));

        if (!mchConverted.IsChanged && !mchChecked.IsChanged)
            return CreateUnchanged(t_expression);

        return CreateChanged(mchChecked.Value);
    }

    auto CreateImplicitlyConvertedAndTypeCheckedOptional(const std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>& t_optExpression, const TypeInfo& t_targetTypeInfo) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>>>
    {
        if (!t_optExpression.has_value())
            return CreateUnchanged(std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>{});

        ACE_TRY(mchConvertedAndChecked, CreateImplicitlyConvertedAndTypeChecked(
            t_optExpression.value(),
            t_targetTypeInfo
        ));

        return MaybeChanged<std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>>
        {
            mchConvertedAndChecked.IsChanged,
            mchConvertedAndChecked.Value
        };
    }
}
