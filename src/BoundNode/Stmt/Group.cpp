#include "BoundNode/Stmt/Group.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt
{
    Group::Group(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>& t_stmts
    ) : m_Scope{ t_scope },
        m_Stmts{ t_stmts }
    {
    }

    auto Group::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Group::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto Group::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Group>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmt)
        {
            return t_stmt->GetOrCreateTypeCheckedStmt({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (!mchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Group>(
            m_Scope,
            mchCheckedContent.Value
        );

        return CreateChanged(returnValue);
    }

    auto Group::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Group::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Group>>
    {
        const auto mchLoweredStmts = TransformMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmt)
        {
            return t_stmt->GetOrCreateLoweredStmt({});
        });

        if (!mchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Group>(
            m_Scope,
            mchLoweredStmts.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Group::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Group::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
    
    auto Group::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return m_Stmts;
    }
}
