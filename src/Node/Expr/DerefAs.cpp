#include "Node/Expr/DerefAs.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNode/Expr/DerefAs.hpp"

namespace Ace::Node::Expr
{
    DerefAs::DerefAs(
        const TypeName& t_typeName, 
        const std::shared_ptr<const Node::Expr::IBase>& t_expr
    ) : m_TypeName{ t_typeName },
        m_Expr{ t_expr }
    {
    }

    auto DerefAs::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAs::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAs::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::DerefAs>
    {
        return std::make_shared<const Node::Expr::DerefAs>(
            m_TypeName,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto DerefAs::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto DerefAs::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::DerefAs>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::make_shared<const BoundNode::Expr::DerefAs>(
            boundExpr,
            typeSymbol
        );
    }

    auto DerefAs::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
