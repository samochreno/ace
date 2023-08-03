#include "Nodes/Exprs/DerefAsExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"

namespace Ace
{
    DerefAsExprNode::DerefAsExprNode(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto DerefAsExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefAsExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAsExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const DerefAsExprNode>
    {
        return std::make_shared<const DerefAsExprNode>(
            m_SrcLocation,
            m_TypeName,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto DerefAsExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto DerefAsExprNode::CreateBound() const -> std::shared_ptr<const DerefAsExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        const auto boundExpr = m_Expr->CreateBoundExpr();

        const auto expTypeSymbol = GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnostics.Add(expTypeSymbol);

        auto* const typeSymbol = expTypeSymbol.UnwrapOr(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return std::make_shared<const DerefAsExprBoundNode>(
            diagnostics,
            GetSrcLocation(),
            boundExpr,
            typeSymbol
        );
    }

    auto DerefAsExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
