#include "Node/Expression/LiteralSymbol.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "BoundNode/Expression/VariableReference/Static.hpp"
#include "Symbol/Variable/Base.hpp"

namespace Ace::Node::Expression
{
    auto LiteralSymbol::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto LiteralSymbol::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::LiteralSymbol>
    {
        return std::make_shared<const Node::Expression::LiteralSymbol>(
            t_scope,
            m_Name
        );
    }

    auto LiteralSymbol::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::VariableReference::Static>>
    {
        ACE_TRY(variableSymbol, m_Scope->ResolveStaticSymbol<Symbol::Variable::IBase>(m_Name));
        return std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            m_Scope,
            variableSymbol
            );
    }
}
