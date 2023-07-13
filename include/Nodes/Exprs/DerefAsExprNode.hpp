#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class DerefAsExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<DerefAsExprNode>,
        public virtual IBindableNode<DerefAsExprBoundNode>
    {
    public:
        DerefAsExprNode(
            const SourceLocation& t_sourceLocation,
            const TypeName& t_typeName, 
            const std::shared_ptr<const IExprNode>& t_expr
        );
        virtual ~DerefAsExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const DerefAsExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const DerefAsExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;
        
    private:
        SourceLocation m_SourceLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
