#include "BoundNodes/Exprs/OrExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    OrExprBoundNode::OrExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto OrExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto OrExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto OrExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto OrExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const OrExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedLHSExpr = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            typeInfo
        ));
        const auto checkedRHSExpr = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            typeInfo
        ));

        if (
            (checkedLHSExpr == m_LHSExpr) &&
            (checkedRHSExpr == m_RHSExpr)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const OrExprBoundNode>(
                GetSrcLocation(),
                checkedLHSExpr,
                checkedRHSExpr
            ),
            diagnostics,
        };
    }

    auto OrExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto OrExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const OrExprBoundNode>
    {
        const auto loweredLHSExpr = m_LHSExpr->CreateLoweredExpr({});
        const auto loweredRHSExpr = m_RHSExpr->CreateLoweredExpr({});

        if (
            (loweredLHSExpr == m_LHSExpr) &&
            (loweredRHSExpr == m_RHSExpr)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const OrExprBoundNode>(
            GetSrcLocation(),
            loweredLHSExpr,
            loweredRHSExpr
        )->CreateLowered({});
    }

    auto OrExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto OrExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const boolType = GetCompilation()->GetNatives().Bool.GetIRType(
            emitter.GetContext()
        );

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(boolType);

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

        emitter.GetBlockBuilder().Builder.CreateStore(
            lhsLoadInst,
            allocaInst
        );

        auto falseBlockBuilder = std::make_unique<BlockBuilder>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        auto endBlockBuilder = std::make_unique<BlockBuilder>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        emitter.GetBlockBuilder().Builder.CreateCondBr(
            lhsLoadInst,
            endBlockBuilder->Block,
            falseBlockBuilder->Block
        );

        emitter.SetBlockBuilder(std::move(falseBlockBuilder));

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

    auto OrExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R 
        };
    }
}
