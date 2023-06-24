#include "BoundNode/Expr/Box.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Template/Function.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    Box::Box(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto Box::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto Box::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Box::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::Box>>> 
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::Box>(
            mchCheckedExpr.Value
        );
        return CreateChanged(returnValue);
    }
    
    auto Box::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Box::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Static>> 
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        const auto symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__new.GetSymbol(),
            std::nullopt,
            { mchLoweredExpr.Value->GetTypeInfo().Symbol },
            {}
        ).Unwrap();
        
        auto* const functionSymbol = dynamic_cast<Symbol::Function*>(symbol);
        ACE_ASSERT(functionSymbol);

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpr.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Box::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Box::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto Box::GetTypeInfo() const -> TypeInfo
    {
        return
        { 
            m_Expr->GetTypeInfo().Symbol->GetWithStrongPointer(),
            ValueKind::R
        };
    }
}
