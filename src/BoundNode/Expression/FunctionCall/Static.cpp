#include "BoundNode/Expression/FunctionCall/Static.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expression::FunctionCall
{
    auto Static::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Arguments);

        return children;
    }

    auto Static::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>>
    {

        const auto argumentTypeInfos = m_FunctionSymbol->CollectArgumentTypeInfos();
        ACE_TRY_ASSERT(m_Arguments.size() == argumentTypeInfos.size());

        ACE_TRY(mchConvertedAndCheckedArguments, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Arguments,
            argumentTypeInfos
        ));

        if (!mchConvertedAndCheckedArguments.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            m_Scope,
            m_FunctionSymbol,
            mchConvertedAndCheckedArguments.Value
        );
        return CreateChanged(returnValue);
    }

    auto Static::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>
    {
        const auto mchLoweredArguments = TransformMaybeChangedVector(m_Arguments,
        [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
        {
            return t_argument->GetOrCreateLoweredExpression({});
        });

        if (!mchLoweredArguments.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            m_Scope,
            m_FunctionSymbol,
            mchLoweredArguments.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Static::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        std::vector<llvm::Value*> arguments{};
        std::transform(begin(m_Arguments), end(m_Arguments), back_inserter(arguments),
        [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
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
    
    auto Static::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
