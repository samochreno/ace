#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    DerefAsExprBoundNode::DerefAsExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        ITypeSymbol* const t_typeSymbol
    ) : m_TypeSymbol{ t_typeSymbol },
        m_Expr{ t_expr }
    {
    }

    auto DerefAsExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAsExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const DerefAsExprBoundNode>>>
    {
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol->GetUnaliased();

        ACE_TRY_ASSERT(
            (typeSymbol == GetCompilation()->Natives->Pointer.GetSymbol()) ||
            (typeSymbol->IsReference())
        );

        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const DerefAsExprBoundNode>(
            m_Expr,
            m_TypeSymbol
        );
        return CreateChanged(returnValue);
    }

    auto DerefAsExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto DerefAsExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const DerefAsExprBoundNode>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const DerefAsExprBoundNode>(
            m_Expr,
            m_TypeSymbol
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto DerefAsExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto DerefAsExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );

        auto* const pointerType = GetCompilation()->Natives->Pointer.GetIRType();

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            pointerType,
            exprEmitResult.Value
        );

        return { loadInst, temporaries };
    }

    auto DerefAsExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
