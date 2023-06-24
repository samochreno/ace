#include "BoundNode/Expression/Box.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Template/Function.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    Box::Box(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression
    ) : m_Expression{ t_expression }
    {
    }

    auto Box::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expression->GetScope();
    }

    auto Box::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Box::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Box>>> 
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expression::Box>(
            mchCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }
    
    auto Box::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Box::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>> 
    {
        const auto mchLoweredExpression =
            m_Expression->GetOrCreateLoweredExpression({});

        const auto symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__new.GetSymbol(),
            std::nullopt,
            { mchLoweredExpression.Value->GetTypeInfo().Symbol },
            {}
        ).Unwrap();
        
        auto* const functionSymbol = dynamic_cast<Symbol::Function*>(symbol);
        ACE_ASSERT(functionSymbol);

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpression.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Box::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Box::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto Box::GetTypeInfo() const -> TypeInfo
    {
        return
        { 
            m_Expression->GetTypeInfo().Symbol->GetWithStrongPointer(),
            ValueKind::R
        };
    }
}
