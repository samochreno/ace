#include "Nodes/Stmts/CopyStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/CopyStmtBoundNode.hpp"

namespace Ace
{
    CopyStmtNode::CopyStmtNode(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprNode>& srcExpr,
        const std::shared_ptr<const IExprNode>& dstExpr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_SrcExpr{ srcExpr },
        m_DstExpr{ dstExpr }
    {
    }

    auto CopyStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CopyStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SrcExpr->GetScope();
    }

    auto CopyStmtNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_SrcExpr);
        AddChildren(children, m_DstExpr);

        return children;
    }

    auto CopyStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const CopyStmtNode>
    {
        return std::make_shared<const CopyStmtNode>(
            m_SrcLocation,
            m_TypeName,
            m_SrcExpr->CloneInScopeExpr(scope),
            m_DstExpr->CloneInScopeExpr(scope)
        );
    }

    auto CopyStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto CopyStmtNode::CreateBound() const -> Diagnosed<std::shared_ptr<const CopyStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundSrcExpr =
            diagnostics.Collect(m_SrcExpr->CreateBoundExpr());
        const auto boundDstExpr =
            diagnostics.Collect(m_DstExpr->CreateBoundExpr());

        const auto typeName = m_TypeName.ToSymbolName(GetCompilation());
        const auto optTypeSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<ISizedTypeSymbol>(typeName)
        );

        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed
        {
            std::make_shared<const CopyStmtBoundNode>(
                GetSrcLocation(),
                typeSymbol,
                boundSrcExpr,
                boundDstExpr
            ),
            std::move(diagnostics),
        };
    }

    auto CopyStmtNode::CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
