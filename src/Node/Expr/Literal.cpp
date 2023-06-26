#include "Node/Expr/Literal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/Literal.hpp"

namespace Ace::Node::Expr
{
    Literal::Literal(
        const std::shared_ptr<Scope>& t_scope,
        const LiteralKind& t_kind,
        const std::string& t_string
    ) : m_Scope{ t_scope },
        m_Kind{ t_kind },
        m_String{ t_string }
    {
    }

    auto Literal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Literal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Literal::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::Literal>
    {
        return std::make_shared<const Node::Expr::Literal>(
            t_scope,
            m_Kind,
            m_String
        );
    }

    auto Literal::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Literal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Literal>>
    {
        return std::make_shared<const BoundNode::Expr::Literal>(
            m_Scope, 
            m_Kind,
            m_String
        );
    }

    auto Literal::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
