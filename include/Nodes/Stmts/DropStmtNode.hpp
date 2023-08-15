#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/DropStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class DropStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableInScopeNode<DropStmtNode>,
        public virtual IBindableNode<DropStmtBoundNode>
    {
    public:
        DropStmtNode(
            const SrcLocation& srcLocation,
            const TypeName& typeName, 
            const std::shared_ptr<const IExprNode>& expr
        );
        virtual ~DropStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const DropStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Diagnosed<std::shared_ptr<const DropStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
