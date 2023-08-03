#include "Nodes/Exprs/CastExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    CastExprNode::CastExprNode(
        const SrcLocation& srcLocation,
        const TypeName& typeName,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto CastExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CastExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto CastExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto CastExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const CastExprNode>
    {
        return std::make_shared<const CastExprNode>(
            m_SrcLocation,
            m_TypeName,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto CastExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto CastExprNode::CreateBound() const -> std::shared_ptr<const IExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        const auto boundExpr = m_Expr->CreateBoundExpr();

        const auto expTypeSymbol = GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnostics.Add(expTypeSymbol);

        auto* const typeSymbol = expTypeSymbol.UnwrapOr(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        const auto expCchConvertedBoundExpr = CreateExplicitlyConverted(
            boundExpr, 
            TypeInfo{ typeSymbol, ValueKind::R }
        );
        diagnostics.Add(expCchConvertedBoundExpr);

        return expCchConvertedBoundExpr.Unwrap().Value->CloneWithDiagnosticsExpr(
            diagnostics
        );
    }

    auto CastExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
