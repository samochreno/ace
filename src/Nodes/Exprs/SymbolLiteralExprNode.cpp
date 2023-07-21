#include "Nodes/Exprs/SymbolLiteralExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"
#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    SymbolLiteralExprNode::SymbolLiteralExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_name
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto SymbolLiteralExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto SymbolLiteralExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SymbolLiteralExprNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};

    }

    auto SymbolLiteralExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const SymbolLiteralExprNode>
    {
        return std::make_shared<const SymbolLiteralExprNode>(
            m_SourceLocation,
            t_scope,
            m_Name
        );
    }

    auto SymbolLiteralExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto SymbolLiteralExprNode::CreateBound() const -> Expected<std::shared_ptr<const StaticVarReferenceExprBoundNode>>
    {
        ACE_TRY(varSymbol, m_Scope->ResolveStaticSymbol<IVarSymbol>(m_Name));
        return std::make_shared<const StaticVarReferenceExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            varSymbol
        );
    }

    auto SymbolLiteralExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }

    auto SymbolLiteralExprNode::GetName() const -> const SymbolName&
    {
        return m_Name;
    }
}
