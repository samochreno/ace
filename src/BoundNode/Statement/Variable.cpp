#include "BoundNode/Statement/Variable.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/VariableReference/Static.hpp"
#include "BoundNode/Statement/Assignment/Normal.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    Variable::Variable(
        Symbol::Variable::Local* const t_symbol,
        const std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>& t_optAssignedExpression
    ) : m_Symbol{ t_symbol },
        m_OptAssignedExpression{ t_optAssignedExpression }
    {
    }

    auto Variable::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Variable::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        if (m_OptAssignedExpression.has_value())
        {
            AddChildren(children, m_OptAssignedExpression.value());
        }

        return children;
    }

    auto Variable::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Variable>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(mchConvertedAndCheckedOptAssignedExpression, CreateImplicitlyConvertedAndTypeCheckedOptional(
            m_OptAssignedExpression,
            TypeInfo{ m_Symbol->GetType(), ValueKind::R }
        ));

        if (!mchConvertedAndCheckedOptAssignedExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Variable>(
            m_Symbol,
            mchConvertedAndCheckedOptAssignedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Variable::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Variable::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Variable>>
    {
        const auto mchLoweredOptAssignedExpression = TransformMaybeChangedOptional(m_OptAssignedExpression,
        [](const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)
        {
            return t_expression->GetOrCreateLoweredExpression({});
        });

        if (!mchLoweredOptAssignedExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Variable>(
            m_Symbol,
            mchLoweredOptAssignedExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Variable::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Variable::Emit(Emitter& t_emitter) const -> void
    {
        if (!m_OptAssignedExpression.has_value())
        {
            return;
        }

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

    auto Variable::GetSymbol() const -> Symbol::Variable::Local*
    {
        return m_Symbol;
    }
}
