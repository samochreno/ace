#include "Node/Stmt/While.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "Node/Stmt/Block.hpp"
#include "BoundNode/Stmt/While.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    While::While(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const Node::Expr::IBase>& t_condition,
        const std::shared_ptr<const Node::Stmt::Block>& t_body
    ) : m_Scope{ t_scope },
        m_Condition{ t_condition },
        m_Body{ t_body }
    {
    }

    auto While::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto While::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto While::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::While>
    {
        return std::make_shared<const Node::Stmt::While>(
            t_scope,
            m_Condition->CloneInScopeExpr(t_scope),
            m_Body->CloneInScope(t_scope)
        );
    }

    auto While::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto While::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::While>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        ACE_TRY(boundBody, m_Body->CreateBound());
        return std::make_shared<const BoundNode::Stmt::While>(
            m_Scope,
            boundCondition,
            boundBody
        );
    }

    auto While::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
