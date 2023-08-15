#include "Nodes/Stmts/DropStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/DropStmtBoundNode.hpp"

namespace Ace
{
    DropStmtNode::DropStmtNode(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_Expr{ expr }
    {
    }

    auto DropStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DropStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DropStmtNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DropStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const DropStmtNode>
    {
        return std::make_shared<const DropStmtNode>(
            m_SrcLocation,
            m_TypeName,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto DropStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto DropStmtNode::CreateBound() const -> Diagnosed<std::shared_ptr<const DropStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundExpr = diagnostics.Collect(m_Expr->CreateBoundExpr());

        const auto typeName = m_TypeName.ToSymbolName(GetCompilation());
        const auto optTypeSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<ISizedTypeSymbol>(typeName)
        );

        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed
        {
            std::make_shared<const DropStmtBoundNode>(
                GetSrcLocation(),
                typeSymbol,
                boundExpr
            ),
            std::move(diagnostics),
        };
    }

    auto DropStmtNode::CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
