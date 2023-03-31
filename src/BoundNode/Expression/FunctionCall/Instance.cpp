#include "BoundNode/Expression/FunctionCall/Instance.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression//Reference.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expression::FunctionCall
{
    auto Instance::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);
        AddChildren(children, m_Arguments);

        return children;
    }

    auto Instance::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Instance>>>
    {
        ACE_TRY_ASSERT(m_Arguments.size() == m_FunctionSymbol->GetParameters().size());

        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        ACE_TRY(mchConvertedAndCheckedArguments, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Arguments,
            m_FunctionSymbol->GetArgumentTypeInfos()
        ));

        if (
            !mchCheckedExpression.IsChanged &&
            !mchConvertedAndCheckedArguments.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Instance>(
            mchCheckedExpression.Value,
            m_FunctionSymbol,
            mchConvertedAndCheckedArguments.Value
            );

        return CreateChanged(returnValue);
    }

    auto Instance::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Instance>>>
    {
        ACE_TRY(mchLoweredExpression, m_Expression->GetOrCreateLoweredExpression({}));

        ACE_TRY(mchLoweredArguments, TransformExpectedMaybeChangedVector(m_Arguments, [&]
        (const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
        {
            return t_argument->GetOrCreateLoweredExpression({});
        }));

        if (
            !mchLoweredExpression.IsChanged && 
            !mchLoweredArguments.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Instance>(
            mchLoweredExpression.Value,
            m_FunctionSymbol,
            mchLoweredArguments.Value
            );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }

    auto Instance::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        std::vector<llvm::Value*> arguments{};

        const auto expression = [&]() -> std::shared_ptr<const BoundNode::Expression::IBase>
        {
            if (m_Expression->GetTypeInfo().Symbol->IsReference())
            {
                return m_Expression;
            }

            return std::make_shared<const BoundNode::Expression::Reference>(m_Expression);
        }();

        auto* const selfType = t_emitter.GetIRType(expression->GetTypeInfo().Symbol);
        
        const auto selfEmitResult = expression->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(selfEmitResult.Temporaries), end(selfEmitResult.Temporaries));

        arguments.push_back(selfEmitResult.Value);

        std::transform(begin(m_Arguments), end(m_Arguments), back_inserter(arguments), [&]
        (const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
        {
            const auto argumentEmitResult = t_argument->Emit(t_emitter);
            temporaries.insert(end(temporaries), begin(argumentEmitResult.Temporaries), end(argumentEmitResult.Temporaries));
            return argumentEmitResult.Value;
        });

        auto* const callInst = t_emitter.GetBlockBuilder().Builder.CreateCall(
            t_emitter.GetFunctionMap().at(m_FunctionSymbol),
            arguments
        );

        if (callInst->getType()->isVoidTy())
        {
            return { nullptr, temporaries };
        }

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(callInst->getType());
        temporaries.emplace_back(allocaInst, m_FunctionSymbol->GetType());

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            callInst,
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto Instance::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
