#include "BoundNode/Statement/Assert.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "BoundNode/Statement/If.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Statement/Exit.hpp"
#include "BoundNode/Expression//LogicalNegation.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Statement
{
    auto Assert::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Assert::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assert>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation().Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (!mchConvertedAndCheckedCondition.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Assert>(mchConvertedAndCheckedCondition.Value);

        return CreateChanged(returnValue);
    }

    auto Assert::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>>
    {
        ACE_TRY(mchLoweredCondition, m_Condition->GetOrCreateLoweredExpression({}));

        const auto condition = std::make_shared<const BoundNode::Expression::LogicalNegation>(
            mchLoweredCondition.Value
        );

        const auto bodyScope = GetScope()->GetOrCreateChild({});

        const auto exitStatement = std::make_shared<const BoundNode::Statement::Exit>(
            bodyScope
        );

        const auto bodyStatement = std::make_shared<const BoundNode::Statement::Block>(
            bodyScope,
            std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>{ exitStatement }
        );

        const auto returnValue = std::make_shared<const BoundNode::Statement::If>(
            GetScope(),
            std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>{ condition },
            std::vector<std::shared_ptr<const BoundNode::Statement::Block>>{ bodyStatement }
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }
}
