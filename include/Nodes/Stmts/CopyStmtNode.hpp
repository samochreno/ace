#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/CopyStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CopyStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableInScopeNode<CopyStmtNode>,
        public virtual IBindableNode<CopyStmtBoundNode>
    {
    public:
        CopyStmtNode(
            const SrcLocation& srcLocation,
            const TypeName& typeName, 
            const std::shared_ptr<const IExprNode>& srcExpr,
            const std::shared_ptr<const IExprNode>& dstExpr
        );
        virtual ~CopyStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const CopyStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Diagnosed<std::shared_ptr<const CopyStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprNode> m_SrcExpr{};
        std::shared_ptr<const IExprNode> m_DstExpr{};
    };
}
