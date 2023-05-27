#include "BoundNode/Expression/UnaryUser.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Expression
{
    auto UnaryUser::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto UnaryUser::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::UnaryUser>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::UnaryUser>(
            mchCheckedExpression.Value,
            m_OperatorSymbol
            );

        return CreateChanged(returnValue);
    }

    auto UnaryUser::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>>
    {
        ACE_TRY(mchLoweredExpression, m_Expression->GetOrCreateLoweredExpression({}));

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            GetScope(),
            m_OperatorSymbol,
            std::vector{ mchLoweredExpression.Value }
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }

    auto UnaryUser::GetTypeInfo() const -> TypeInfo
    {
        return { m_OperatorSymbol->GetType(), ValueKind::R };
    }
}
