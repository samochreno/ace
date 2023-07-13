#include "Nodes/Exprs/LiteralExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/LiteralExprBoundNode.hpp"

namespace Ace
{
    LiteralExprNode::LiteralExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const LiteralKind t_kind,
        const std::string& t_string
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Kind{ t_kind },
        m_String{ t_string }
    {
    }

    auto LiteralExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto LiteralExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralExprNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto LiteralExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const LiteralExprNode>
    {
        return std::make_shared<const LiteralExprNode>(
            m_SourceLocation,
            t_scope,
            m_Kind,
            m_String
        );
    }

    auto LiteralExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto LiteralExprNode::CreateBound() const -> Expected<std::shared_ptr<const LiteralExprBoundNode>>
    {
        return std::make_shared<const LiteralExprBoundNode>(
            m_Scope, 
            m_Kind,
            m_String
        );
    }

    auto LiteralExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
