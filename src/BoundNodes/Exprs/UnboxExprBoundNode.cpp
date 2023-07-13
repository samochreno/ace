#include "BoundNodes/Exprs/UnboxExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    UnboxExprBoundNode::UnboxExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto UnboxExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);
        
        return children;
    }

    auto UnboxExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const UnboxExprBoundNode>>>
    {
        ACE_TRY_ASSERT(m_Expr->GetTypeInfo().Symbol->IsStrongPointer());

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__value.GetSymbol(),
            std::nullopt,
            { m_Expr->GetTypeInfo().Symbol->GetWithoutReference()->GetWithoutStrongPointer() },
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

        return CreateChanged(std::make_shared<const UnboxExprBoundNode>(
            mchCheckedAndConvertedExpr.Value
        ));
    }

    auto UnboxExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto UnboxExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__value.GetSymbol(),
            std::nullopt,
            { mchLoweredExpr.Value->GetTypeInfo().Symbol->GetWithoutReference()->GetWithoutStrongPointer() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpr.Value }
        )->GetOrCreateLowered({}).Value);
    }

    auto UnboxExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto UnboxExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UnboxExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            m_Expr->GetTypeInfo().Symbol->GetWithoutReference()->GetWithoutStrongPointer(),
            ValueKind::R,
        };
    }
}
