#include "Nodes/Exprs/DerefAsExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"

namespace Ace
{
    DerefAsExprNode::DerefAsExprNode(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto DerefAsExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefAsExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAsExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const DerefAsExprNode>
    {
        return std::make_shared<const DerefAsExprNode>(
            m_SrcLocation,
            m_TypeName,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto DerefAsExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto DerefAsExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const DerefAsExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundExpr = diagnostics.Collect(m_Expr->CreateBoundExpr());

        const auto optTypeSymbol = diagnostics.Collect(GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed
        {
            std::make_shared<const DerefAsExprBoundNode>(
                GetSrcLocation(),
                boundExpr,
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto DerefAsExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
