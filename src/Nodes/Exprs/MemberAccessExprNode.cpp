#include "Nodes/Exprs/MemberAccessExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/Exprs/VarReferences/InstanceVarReferenceExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    MemberAccessExprNode::MemberAccessExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr,
        const SymbolNameSection& t_name
    ) : m_SourceLocation{ t_sourceLocation },
        m_Expr{ t_expr },
        m_Name{ t_name }
    {
    }

    auto MemberAccessExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto MemberAccessExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto MemberAccessExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto MemberAccessExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const MemberAccessExprNode>
    {
        return std::make_shared<const MemberAccessExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(t_scope),
            m_Name
        );
    }

    auto MemberAccessExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto MemberAccessExprNode::CreateBound() const -> Expected<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        ACE_TRY_ASSERT(m_Name.TemplateArgs.empty());
        ACE_TRY(memberSymbol, GetScope()->ResolveInstanceSymbol<InstanceVarSymbol>(
            boundExpr->GetTypeInfo().Symbol->GetWithoutReference(),
            m_Name
        ));

        return std::make_shared<const InstanceVarReferenceExprBoundNode>(
            GetSourceLocation(),
            boundExpr,
            memberSymbol
        );
    }

    auto MemberAccessExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }

    auto MemberAccessExprNode::GetExpr() const -> const IExprNode*
    {
        return m_Expr.get();
    }

    auto MemberAccessExprNode::GetName() const -> const SymbolNameSection&
    {
        return m_Name;
    }
}
