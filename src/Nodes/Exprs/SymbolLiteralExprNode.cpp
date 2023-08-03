#include "Nodes/Exprs/SymbolLiteralExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/VarRefs/StaticVarRefExprBoundNode.hpp"
#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    SymbolLiteralExprNode::SymbolLiteralExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& name
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto SymbolLiteralExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SymbolLiteralExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SymbolLiteralExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        return {};

    }

    auto SymbolLiteralExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const SymbolLiteralExprNode>
    {
        return std::make_shared<const SymbolLiteralExprNode>(
            m_SrcLocation,
            scope,
            m_Name
        );
    }

    auto SymbolLiteralExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto SymbolLiteralExprNode::CreateBound() const -> std::shared_ptr<const StaticVarRefExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        const auto expVarSymbol = m_Scope->ResolveStaticSymbol<IVarSymbol>(
            m_Name
        );
        diagnostics.Add(expVarSymbol);

        auto* const varSymbol = expVarSymbol.UnwrapOr(
            GetCompilation()->GetErrorSymbols().GetVar()
        );

        return std::make_shared<const StaticVarRefExprBoundNode>(
            diagnostics,
            GetSrcLocation(),
            GetScope(),
            varSymbol
        );
    }

    auto SymbolLiteralExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }

    auto SymbolLiteralExprNode::GetName() const -> const SymbolName&
    {
        return m_Name;
    }
}
