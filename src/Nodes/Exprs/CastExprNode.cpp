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

    auto CastExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundExpr = diagnostics.Collect(m_Expr->CreateBoundExpr());

        const auto optTypeSymbol = diagnostics.Collect(GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        const auto convertedExpr = diagnostics.Collect(CreateExplicitlyConverted(
            boundExpr,
            TypeInfo{ typeSymbol, ValueKind::R }
        ));

        return Diagnosed
        {
            convertedExpr,
            std::move(diagnostics),
        };
    }

    auto CastExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
