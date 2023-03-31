#include "Node/Expression/DerefAs.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "Symbol/Type/Base.hpp"
#include "BoundNode/Expression/DerefAs.hpp"

namespace Ace::Node::Expression
{
    auto DerefAs::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto DerefAs::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::DerefAs>
    {
        return std::make_unique<const Node::Expression::DerefAs>(
            m_TypeName,
            m_Expression->CloneInScopeExpression(t_scope)
        );
    }

    auto DerefAs::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::DerefAs>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<Symbol::Type::IBase>(m_TypeName.ToSymbolName()));
        return std::make_shared<const BoundNode::Expression::DerefAs>(
            boundExpression,
            typeSymbol
        );
    }
}
