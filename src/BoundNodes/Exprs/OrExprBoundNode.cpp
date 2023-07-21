#include "BoundNodes/Exprs/OrExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    OrExprBoundNode::OrExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_SourceLocation{ sourceLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto OrExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto OrExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto OrExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto OrExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const OrExprBoundNode>>>
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

        return CreateChanged(std::make_shared<const OrExprBoundNode>(
            GetSourceLocation(),
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value
        ));
    }

    auto OrExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto OrExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const OrExprBoundNode>>
    {
        const auto mchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto mchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        if (
            !mchLoweredLHSExpr.IsChanged &&
            !mchLoweredRHSExpr.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        return CreateChanged(std::make_shared<const OrExprBoundNode>(
            GetSourceLocation(),
            mchLoweredLHSExpr.Value,
            mchLoweredRHSExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto OrExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto OrExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        auto* const boolType = GetCompilation()->Natives->Bool.GetIRType();

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(boolType);

        const auto lhsEmitResult = m_LHSExpr->Emit(emitter);
        temporaries.insert(
            end(temporaries), 
            begin(lhsEmitResult.Temporaries), 
            end  (lhsEmitResult.Temporaries)
        );

        auto* const lhsLoadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            lhsEmitResult.Value
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            lhsLoadInst,
            allocaInst
        );

        auto falseBlockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            emitter.GetFunction()
        );

        auto endBlockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            emitter.GetFunction()
        );

        emitter.GetBlockBuilder().Builder.CreateCondBr(
            lhsLoadInst,
            endBlockBuilder->Block,
            falseBlockBuilder->Block
        );

        emitter.SetBlockBuilder(std::move(falseBlockBuilder));

        const auto rhsEmitResult = m_RHSExpr->Emit(emitter);
        temporaries.insert(
            end(temporaries), 
            begin(rhsEmitResult.Temporaries), 
            end  (rhsEmitResult.Temporaries)
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

        return { allocaInst, temporaries };
    }

    auto OrExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R 
        };
    }
}
