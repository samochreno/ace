#include "BoundNode/Expression//And.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    auto And::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto And::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::And>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation().Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedLHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpression,
            typeInfo
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpression,
            typeInfo
        ));

        if (
            !mchConvertedAndCheckedLHSExpression.IsChanged &&
            !mchConvertedAndCheckedRHSExpression.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::And>(
            mchConvertedAndCheckedLHSExpression.Value,
            mchConvertedAndCheckedRHSExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto And::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::And>>
    {
        const auto mchLoweredLHSExpression = m_LHSExpression->GetOrCreateLoweredExpression({});
        const auto mchLoweredRHSExpression = m_RHSExpression->GetOrCreateLoweredExpression({});

        if (
            !mchLoweredLHSExpression.IsChanged && 
            !mchLoweredRHSExpression.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::And>(
            mchLoweredLHSExpression.Value,
            mchLoweredRHSExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto And::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        auto* const boolType = GetCompilation().Natives->Bool.GetIRType();

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(boolType);

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            llvm::ConstantInt::get(boolType, 0),
            allocaInst
        );

        const auto lhsEmitResult = m_LHSExpression->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(lhsEmitResult.Temporaries), end(lhsEmitResult.Temporaries));

        auto* const lhsLoadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            lhsEmitResult.Value
        );

        auto trueBlockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation().LLVMContext,
            t_emitter.GetFunction()
            );

        auto endBlockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation().LLVMContext,
            t_emitter.GetFunction()
            );

        t_emitter.GetBlockBuilder().Builder.CreateCondBr(
            lhsLoadInst,
            trueBlockBuilder->Block,
            endBlockBuilder->Block
        );

        t_emitter.SetBlockBuilder(std::move(trueBlockBuilder));

        const auto rhsEmitResult = m_RHSExpression->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(rhsEmitResult.Temporaries), end(rhsEmitResult.Temporaries));

        auto* const rhsLoadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            rhsEmitResult.Value
        );

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            rhsLoadInst,
            allocaInst
        );

        t_emitter.GetBlockBuilder().Builder.CreateBr(endBlockBuilder->Block);

        t_emitter.SetBlockBuilder(std::move(endBlockBuilder));

        return { allocaInst, temporaries };
    }

    auto And::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation().Natives->Bool.GetSymbol(),
            ValueKind::R
        };
    }
}
