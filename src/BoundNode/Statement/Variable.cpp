#include "BoundNode/Statement/Variable.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/VariableReference/Static.hpp"
#include "BoundNode/Statement/Assignment/Normal.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "NativeSymbol.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    auto Variable::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        if (m_OptAssignedExpression.has_value())
        {
            AddChildren(children, m_OptAssignedExpression.value());
        }

        return children;
    }

    auto Variable::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Variable>>>
    {
        ACE_TRY_ASSERT(m_Symbol->GetType() != NativeSymbol::Void.GetSymbol());

        ACE_TRY(mchConvertedAndCheckedOptAssignedExpression, CreateImplicitlyConvertedAndTypeCheckedOptional(
            m_OptAssignedExpression,
            TypeInfo{ m_Symbol->GetType(), ValueKind::R}
        ));

        if (!mchConvertedAndCheckedOptAssignedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Variable>(
            m_Symbol,
            mchConvertedAndCheckedOptAssignedExpression.Value
            );

        return CreateChanged(returnValue);
    }

    auto Variable::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Variable>>>
    {
        ACE_TRY(mchLoweredOptAssignedExpression, TransformExpectedMaybeChangedOptional(m_OptAssignedExpression, [&]
        (const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)
        {
            return t_expression->GetOrCreateLoweredExpression({});
        }));

        if (!mchLoweredOptAssignedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Variable>(
            m_Symbol,
            mchLoweredOptAssignedExpression.Value
            );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered(t_context));
    }

    auto Variable::Emit(Emitter& t_emitter) const -> void
    {
        if (!m_OptAssignedExpression.has_value())
            return;

        const auto variableReferenceExpression = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions, references can be initialized too.
        const auto assignmentStatement = std::make_shared<const BoundNode::Statement::Assignment::Normal>(
            variableReferenceExpression,
            m_OptAssignedExpression.value()
            );

        assignmentStatement->Emit(t_emitter);
    }
}