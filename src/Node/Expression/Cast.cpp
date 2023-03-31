#include "Node/Expression/Cast.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "TypeInfo.hpp"

namespace Ace::Node::Expression
{
    auto Cast::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Cast::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::Cast>
    {
        return std::make_unique<const Node::Expression::Cast>(
            m_TypeName,
            m_Expression->CloneInScopeExpression(t_scope)
            );
    }

    auto Cast::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());

        ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<Symbol::Type::IBase>(m_TypeName.ToSymbolName()));

        ACE_TRY(mchConvertedBoundExpression, BoundNode::Expression::CreateExplicitlyConverted(
            boundExpression, 
            TypeInfo{ typeSymbol, ValueKind::R }
        ));

        return mchConvertedBoundExpression.Value;
    }
}
