#include "Nodes/Exprs/LiteralSymbolExprNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    LiteralSymbolExprNode::LiteralSymbolExprNode(
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto LiteralSymbolExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralSymbolExprNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};

    }

    auto LiteralSymbolExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const LiteralSymbolExprNode>
    {
        return std::make_shared<const LiteralSymbolExprNode>(
            t_scope,
            m_Name
        );
    }

    auto LiteralSymbolExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto LiteralSymbolExprNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Static>>
    {
        ACE_TRY(variableSymbol, m_Scope->ResolveStaticSymbol<IVarSymbol>(m_Name));
        return std::make_shared<const BoundNode::Expr::VarReference::Static>(
            m_Scope,
            variableSymbol
        );
    }

    auto LiteralSymbolExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }

    auto LiteralSymbolExprNode::GetName() const -> const SymbolName&
    {
        return m_Name;
    }
}
