#include "Nodes/Exprs/LiteralExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/LiteralExprBoundNode.hpp"

namespace Ace
{
    LiteralExprNode::LiteralExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const LiteralKind kind,
        const std::string& string
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Kind{ kind },
        m_String{ string }
    {
    }

    auto LiteralExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LiteralExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto LiteralExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const LiteralExprNode>
    {
        return std::make_shared<const LiteralExprNode>(
            m_SrcLocation,
            scope,
            m_Kind,
            m_String
        );
    }

    auto LiteralExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto LiteralExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const LiteralExprBoundNode>>
    {
        return Diagnosed
        {
            std::make_shared<const LiteralExprBoundNode>(
                GetSrcLocation(),
                GetScope(),
                m_Kind,
                m_String
            ),
            DiagnosticBag::Create(),
        };
    }

    auto LiteralExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
