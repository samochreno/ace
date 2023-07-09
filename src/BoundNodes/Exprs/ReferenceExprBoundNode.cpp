#include "BoundNodes/Exprs/ReferenceExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    ReferenceExprBoundNode::ReferenceExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto ReferenceExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ReferenceExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ReferenceExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ReferenceExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ReferenceExprBoundNode>(
            mchCheckedExpr.Value
        ));
    }

    auto ReferenceExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto ReferenceExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ReferenceExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ReferenceExprBoundNode>(
            mchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto ReferenceExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto ReferenceExprBoundNode::Emit(
        Emitter& t_emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        ); 

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(
            exprEmitResult.Value->getType()
        );
        temporaries.emplace_back(
            allocaInst, m_Expr->GetTypeInfo().Symbol->GetWithReference()
        );

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            exprEmitResult.Value,
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto ReferenceExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        const auto scope = typeSymbol->GetScope();

        return { typeSymbol->GetWithReference(), ValueKind::R };
    }
}
