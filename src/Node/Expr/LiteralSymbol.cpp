#include "Node/Expr/LiteralSymbol.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace::Node::Expr
{
    LiteralSymbol::LiteralSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto LiteralSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralSymbol::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};

    }

    auto LiteralSymbol::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::LiteralSymbol>
    {
        return std::make_shared<const Node::Expr::LiteralSymbol>(
            t_scope,
            m_Name
        );
    }

    auto LiteralSymbol::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto LiteralSymbol::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Static>>
    {
        ACE_TRY(variableSymbol, m_Scope->ResolveStaticSymbol<IVarSymbol>(m_Name));
        return std::make_shared<const BoundNode::Expr::VarReference::Static>(
            m_Scope,
            variableSymbol
        );
    }

    auto LiteralSymbol::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }

    auto LiteralSymbol::GetName() const -> const SymbolName&
    {
        return m_Name;
    }
}
