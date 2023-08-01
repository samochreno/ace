#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"

namespace Ace
{
    class DerefAsExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<DerefAsExprNode>,
        public virtual IBindableNode<DerefAsExprBoundNode>
    {
    public:
        DerefAsExprNode(
            const SrcLocation& srcLocation,
            const TypeName& typeName, 
            const std::shared_ptr<const IExprNode>& expr
        );
        virtual ~DerefAsExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const DerefAsExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> std::shared_ptr<const DerefAsExprBoundNode> final;
        auto CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode> final;
        
    private:
        SrcLocation m_SrcLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
