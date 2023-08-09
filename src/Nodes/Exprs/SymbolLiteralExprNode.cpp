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

    auto SymbolLiteralExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const StaticVarRefExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optVarSymbol =
            diagnostics.Collect(m_Scope->ResolveStaticSymbol<IVarSymbol>(m_Name));

        auto* const varSymbol = optVarSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetVar()
        );

        return Diagnosed
        {
            std::make_shared<const StaticVarRefExprBoundNode>(
                GetSrcLocation(),
                GetScope(),
                varSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto SymbolLiteralExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }

    auto SymbolLiteralExprNode::GetName() const -> const SymbolName&
    {
        return m_Name;
    }
}
