#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    BoxExprBoundNode::BoxExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto BoxExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto BoxExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto BoxExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoxExprBoundNode>>> 
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__new.GetSymbol(),
            std::nullopt,
            { m_Expr->GetTypeInfo().Symbol->GetWithoutReference() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        const TypeInfo typeInfo
        {
            functionSymbol->CollectParams().front()->GetType(),
            ValueKind::R,
        };

        ACE_TRY(mchCheckedAndConvertedExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (!mchCheckedAndConvertedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoxExprBoundNode>(
            mchCheckedAndConvertedExpr.Value
        );
        return CreateChanged(returnValue);
    }
    
    auto BoxExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto BoxExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>> 
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__new.GetSymbol(),
            std::nullopt,
            { mchLoweredExpr.Value->GetTypeInfo().Symbol->GetWithoutReference() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        const auto returnValue = std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpr.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto BoxExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto BoxExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto BoxExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        { 
            m_Expr->GetTypeInfo().Symbol->GetWithoutReference()->GetWithStrongPointer(),
            ValueKind::R
        };
    }
}
