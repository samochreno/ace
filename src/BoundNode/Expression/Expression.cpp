#include "BoundNode/Expression/Expression.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    auto Expression::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Expression::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Expression>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Expression>(
            mchCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Expression::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::Expression>>
    {
        const auto mchLoweredExpression = m_Expression->GetOrCreateLoweredExpression({});

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Expression>(
            mchLoweredExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Expression::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        return m_Expression->Emit(t_emitter);
    }

    auto Expression::GetTypeInfo() const -> TypeInfo
    {
        return m_Expression->GetTypeInfo();
    }
}
