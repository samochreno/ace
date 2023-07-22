#include "BoundNodes/Exprs/AndExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    AndExprBoundNode::AndExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto AndExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AndExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto AndExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto AndExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const AndExprBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            typeInfo
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            typeInfo
        ));

        if (
            !mchConvertedAndCheckedLHSExpr.IsChanged &&
            !mchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AndExprBoundNode>(
            GetSrcLocation(),
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value
        ));
    }

    auto AndExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }


    auto AndExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const AndExprBoundNode>>
    {
        const auto mchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto mchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        if (
            !mchLoweredLHSExpr.IsChanged && 
            !mchLoweredRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AndExprBoundNode>(
            GetSrcLocation(),
            mchLoweredLHSExpr.Value,
            mchLoweredRHSExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto AndExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto AndExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const boolType = GetCompilation()->Natives->Bool.GetIRType();

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(boolType);

        emitter.GetBlockBuilder().Builder.CreateStore(
            llvm::ConstantInt::get(boolType, 0),
            allocaInst
        );

        const auto lhsEmitResult = m_LHSExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(lhsEmitResult.Tmps),
            end  (lhsEmitResult.Tmps)
        );

        auto* const lhsLoadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            lhsEmitResult.Value
        );

        auto trueBlockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            emitter.GetFunction()
        );

        auto endBlockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            emitter.GetFunction()
        );

        emitter.GetBlockBuilder().Builder.CreateCondBr(
            lhsLoadInst,
            trueBlockBuilder->Block,
            endBlockBuilder->Block
        );

        emitter.SetBlockBuilder(std::move(trueBlockBuilder));

        const auto rhsEmitResult = m_RHSExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(rhsEmitResult.Tmps),
            end  (rhsEmitResult.Tmps)
        );

        auto* const rhsLoadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            rhsEmitResult.Value
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            rhsLoadInst,
            allocaInst
        );

        emitter.GetBlockBuilder().Builder.CreateBr(endBlockBuilder->Block);

        emitter.SetBlockBuilder(std::move(endBlockBuilder));

        return { allocaInst, tmps };
    }

    auto AndExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R
        };
    }
}
