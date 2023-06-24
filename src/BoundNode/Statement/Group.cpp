#include "BoundNode/Statement/Group.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement
{
    Group::Group(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>& t_statements
    ) : m_Scope{ t_scope },
        m_Statements{ t_statements }
    {
    }

    auto Group::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Group::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Statements);

        return children;
    }

    auto Group::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Statements,
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            return t_statement->GetOrCreateTypeCheckedStatement({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (!mchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Group>(
            m_Scope,
            mchCheckedContent.Value
        );

        return CreateChanged(returnValue);
    }

    auto Group::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Group::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>
    {
        const auto mchLoweredStatements = TransformMaybeChangedVector(m_Statements,
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            return t_statement->GetOrCreateLoweredStatement({});
        });

        if (!mchLoweredStatements.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Group>(
            m_Scope,
            mchLoweredStatements.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Group::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Group::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
    
    auto Group::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return m_Statements;
    }
}
