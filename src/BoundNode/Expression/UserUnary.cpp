#include "BoundNode/Expression/UserUnary.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Expression
{
    auto UserUnary::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto UserUnary::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::UserUnary>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::UserUnary>(
            mchCheckedExpression.Value,
            m_OperatorSymbol
        );
        return CreateChanged(returnValue);
    }

    auto UserUnary::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>
    {
        const auto mchLoweredExpression = m_Expression->GetOrCreateLoweredExpression({});

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            GetScope(),
            m_OperatorSymbol,
            std::vector{ mchLoweredExpression.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto UserUnary::GetTypeInfo() const -> TypeInfo
    {
        return { m_OperatorSymbol->GetType(), ValueKind::R };
    }
}
