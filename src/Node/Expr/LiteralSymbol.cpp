#include "Node/Expr/LiteralSymbol.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace::Node::Expr
{
    auto LiteralSymbol::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto LiteralSymbol::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::LiteralSymbol>
    {
        return std::make_shared<const Node::Expr::LiteralSymbol>(
            t_scope,
            m_Name
        );
    }

    auto LiteralSymbol::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Static>>
    {
        ACE_TRY(variableSymbol, m_Scope->ResolveStaticSymbol<IVarSymbol>(m_Name));
        return std::make_shared<const BoundNode::Expr::VarReference::Static>(
            m_Scope,
            variableSymbol
            );
    }
}
