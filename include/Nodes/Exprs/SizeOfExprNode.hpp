#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Expr/SizeOf.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class SizeOfExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<SizeOfExprNode>,
        public virtual IBindableNode<BoundNode::Expr::SizeOf>
    {
    public:
        SizeOfExprNode(
            const std::shared_ptr<Scope>& t_scope,
            const TypeName& t_typeName
        );
        virtual ~SizeOfExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const SizeOfExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::SizeOf>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        TypeName m_TypeName{};
    };
}
