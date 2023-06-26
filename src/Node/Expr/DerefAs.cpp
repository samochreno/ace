#include "Node/Expr/DerefAs.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNode/Expr/DerefAs.hpp"

namespace Ace::Node::Expr
{
    auto DerefAs::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAs::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::DerefAs>
    {
        return std::make_shared<const Node::Expr::DerefAs>(
            m_TypeName,
            m_Expr->CloneInScopeExpr(t_scope)
        );
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
}
