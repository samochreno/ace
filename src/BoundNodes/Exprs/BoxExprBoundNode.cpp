#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
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
    BoxExprBoundNode::BoxExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr }
    {
    }

    auto BoxExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const TypeCheckingContext& context
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

        return CreateChanged(std::make_shared<const BoxExprBoundNode>(
            GetSourceLocation(),
            mchCheckedAndConvertedExpr.Value
        ));
    }
    
    auto BoxExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto BoxExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
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

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpr.Value }
        )->GetOrCreateLowered({}).Value);
    }

    auto BoxExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto BoxExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
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
