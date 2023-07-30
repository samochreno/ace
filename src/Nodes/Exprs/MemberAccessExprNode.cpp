#include "Nodes/Exprs/MemberAccessExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Ident.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "BoundNodes/Exprs/VarRefs/InstanceVarRefExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    MemberAccessExprNode::MemberAccessExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr,
        const SymbolNameSection& name
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_Name{ name }
    {
    }

    auto MemberAccessExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto MemberAccessExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto MemberAccessExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto MemberAccessExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const MemberAccessExprNode>
    {
        return std::make_shared<const MemberAccessExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope),
            m_Name
        );
    }

    auto MemberAccessExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto MemberAccessExprNode::CreateBound() const -> Expected<std::shared_ptr<const InstanceVarRefExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        ACE_TRY_ASSERT(m_Name.TemplateArgs.empty());
        ACE_TRY(memberSymbol, GetScope()->ResolveInstanceSymbol<InstanceVarSymbol>(
            boundExpr->GetTypeInfo().Symbol->GetWithoutRef(),
            m_Name
        ));

        return std::make_shared<const InstanceVarRefExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
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
