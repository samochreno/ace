#include "BoundNode/Statement/Expression.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    Expression::Expression(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression
    ) : m_Expression{ t_expression }
    {
    }

    auto Expression::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expression->GetScope();
    }

    auto Expression::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Expression::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Expression>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));
        
        if (!mchCheckedExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Expression>(
            mchCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Expression::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Expression::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Expression>>
    {
        const auto mchLoweredExpression = m_Expression->GetOrCreateLoweredExpression({});

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Expression>(
            mchLoweredExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Expression::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Expression::Emit(Emitter& t_emitter) const -> void
    {
        const auto expressionEmitResult = m_Expression->Emit(t_emitter);
        
        t_emitter.EmitDropTemporaries(expressionEmitResult.Temporaries);
    }
}
