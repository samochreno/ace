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
    auto While::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto While::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::While>
    {
        return std::make_shared<const Node::Stmt::While>(
            t_scope,
            m_Condition->CloneInScopeExpr(t_scope),
            m_Body->CloneInScope(t_scope)
        );
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
}
