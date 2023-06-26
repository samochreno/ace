#include "Node/Expr/Cast.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace::Node::Expr
{
    auto Cast::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Cast::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Cast>
    {
        return std::make_shared<const Node::Expr::Cast>(
            m_TypeName,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto Cast::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        ACE_TRY(mchConvertedBoundExpr, BoundNode::Expr::CreateExplicitlyConverted(
            boundExpr, 
            TypeInfo{ typeSymbol, ValueKind::R }
        ));

        return mchConvertedBoundExpr.Value;
    }
}
