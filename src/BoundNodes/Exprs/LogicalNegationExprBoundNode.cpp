#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    LogicalNegationExprBoundNode::LogicalNegationExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto LogicalNegationExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
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
            mchConvertedAndCheckedExpr.Value
        ));
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            mchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto LogicalNegationExprBoundNode::Emit(
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

        auto* const boolType = GetCompilation()->Natives->Bool.GetIRType();

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            exprEmitResult.Value
        );

        auto* const negatedValue = t_emitter.GetBlockBuilder().Builder.CreateXor(
            loadInst,
            1
        );

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(
            boolType
        );

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            negatedValue,
            allocaInst
        );

        return { allocaInst, exprEmitResult.Temporaries };
    }

    auto LogicalNegationExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { GetCompilation()->Natives->Bool.GetSymbol(), ValueKind::R };
    }
}
