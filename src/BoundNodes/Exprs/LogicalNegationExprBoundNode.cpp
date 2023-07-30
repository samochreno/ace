#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

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
    LogicalNegationExprBoundNode::LogicalNegationExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LogicalNegationExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const LogicalNegationExprBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (!mchConvertedAndCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            GetSrcLocation(),
            mchConvertedAndCheckedExpr.Value
        ));
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            GetSrcLocation(),
            mchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto LogicalNegationExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const boolType = GetCompilation()->Natives->Bool.GetIRType();

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            exprEmitResult.Value
        );

        auto* const negatedValue = emitter.GetBlockBuilder().Builder.CreateXor(
            loadInst,
            1
        );

        auto* const allocaInst = emitter.GetBlockBuilder().Builder.CreateAlloca(
            boolType
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            negatedValue,
            allocaInst
        );

        return { allocaInst, exprEmitResult.Tmps };
    }

    auto LogicalNegationExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { GetCompilation()->Natives->Bool.GetSymbol(), ValueKind::R };
    }
}
