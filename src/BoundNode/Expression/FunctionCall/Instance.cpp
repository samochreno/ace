#include "BoundNode/Expression/FunctionCall/Instance.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression//Reference.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expression::FunctionCall
{
    Instance::Instance(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression,
        Symbol::Function* const t_functionSymbol,
        const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_arguments
    ) : m_Expression{ t_expression },
        m_FunctionSymbol{ t_functionSymbol },
        m_Arguments{ t_arguments }
    {
    }

    auto Instance::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expression->GetScope();
    }

    auto Instance::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);
        AddChildren(children, m_Arguments);

        return children;
    }

    auto Instance::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Instance>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        const auto argumentTypeInfos =
            m_FunctionSymbol->CollectArgumentTypeInfos();
        ACE_TRY_ASSERT(m_Arguments.size() == argumentTypeInfos.size());

        ACE_TRY(mchConvertedAndCheckedArguments, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Arguments,
            m_FunctionSymbol->CollectArgumentTypeInfos()
        ));

        if (
            !mchCheckedExpression.IsChanged &&
            !mchConvertedAndCheckedArguments.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Instance>(
            mchCheckedExpression.Value,
            m_FunctionSymbol,
            mchConvertedAndCheckedArguments.Value
        );
        return CreateChanged(returnValue);
    }

    auto Instance::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Instance::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Instance>>
    {
        const auto mchLoweredExpression =
            m_Expression->GetOrCreateLoweredExpression({});

        const auto mchLoweredArguments = TransformMaybeChangedVector(m_Arguments,
        [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
        {
            return t_argument->GetOrCreateLoweredExpression({});
        });

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
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }


    auto Instance::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        return GetOrCreateLowered(t_context);
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

            return std::make_shared<const BoundNode::Expression::Reference>(
                m_Expression
            );
        }();

        auto* const selfType =
            t_emitter.GetIRType(expression->GetTypeInfo().Symbol);
        
        const auto selfEmitResult = expression->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(selfEmitResult.Temporaries),
            end  (selfEmitResult.Temporaries)
        );

        arguments.push_back(selfEmitResult.Value);

        std::transform(
            begin(m_Arguments),
            end  (m_Arguments),
            back_inserter(arguments),
            [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
            {
                const auto argumentEmitResult = t_argument->Emit(t_emitter);
                temporaries.insert(
                    end(temporaries),
                    begin(argumentEmitResult.Temporaries),
                    end  (argumentEmitResult.Temporaries)
                );
                return argumentEmitResult.Value;
            }
        );

        auto* const callInst = t_emitter.GetBlockBuilder().Builder.CreateCall(
            t_emitter.GetFunctionMap().at(m_FunctionSymbol),
            arguments
        );

        if (callInst->getType()->isVoidTy())
        {
            return { nullptr, temporaries };
        }

        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(callInst->getType());
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
