#include "Node/Stmt/If.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Node/Expr/Base.hpp"
#include "Node/Stmt/Block.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/If.hpp"

namespace Ace::Node::Stmt
{
    If::If(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const Node::Expr::IBase>>& t_conditions,
        const std::vector<std::shared_ptr<const Node::Stmt::Block>>& t_bodies
    ) : m_Scope{ t_scope },
        m_Conditions{ t_conditions },
        m_Bodies{ t_bodies }

    {
    }

    auto If::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto If::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Conditions);
        AddChildren(children, m_Bodies);

        return children;
    }

    auto If::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::If>
    {
        std::vector<std::shared_ptr<const Node::Expr::IBase>> clonedConditions{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(clonedConditions),
            [&](const std::shared_ptr<const Node::Expr::IBase>& t_condition)
            {
                return t_condition->CloneInScopeExpr(t_scope);
            }
        );

        std::vector<std::shared_ptr<const Node::Stmt::Block>> clonedBodies{};
        std::transform(
            begin(m_Bodies),
            end  (m_Bodies),
            back_inserter(clonedBodies),
            [&](const std::shared_ptr<const Node::Stmt::Block>& t_body)
            {
                return t_body->CloneInScope(t_scope);
            }
        );

        return std::make_shared<const Node::Stmt::If>(
            t_scope,
            clonedConditions,
            clonedBodies
        );
    }

    auto If::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto If::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::If>>
    {
        ACE_TRY(boundConditions, TransformExpectedVector(m_Conditions,
        [](const std::shared_ptr<const Node::Expr::IBase>& t_condition)
        {
            return t_condition->CreateBoundExpr();
        }));

        ACE_TRY(boundBodies, TransformExpectedVector(m_Bodies,
        [](const std::shared_ptr<const Node::Stmt::Block>& t_body)
        {
            return t_body->CreateBound();
        }));

        return std::make_shared<const BoundNode::Stmt::If>(
            m_Scope,
            boundConditions,
            boundBodies
        );
    }

    auto If::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
