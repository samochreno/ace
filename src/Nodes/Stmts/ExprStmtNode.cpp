#include "Nodes/Stmts/ExprStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ExprStmtNode::ExprStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto ExprStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ExprStmtNode>
    {
        return std::make_shared<const ExprStmtNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto ExprStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto ExprStmtNode::CreateBound() const -> Expected<std::shared_ptr<const ExprStmtBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const ExprStmtBoundNode>(
            GetSrcLocation(),
            boundExpr
        );
    }

    auto ExprStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
