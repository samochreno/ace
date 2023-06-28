#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class LiteralSymbolExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<LiteralSymbolExprNode>, 
        public virtual IBindableNode<BoundNode::Expr::VarReference::Static>
    {
    public:
        LiteralSymbolExprNode(
            const std::shared_ptr<Scope>& t_scope,
            const SymbolName& t_name
        );
        virtual ~LiteralSymbolExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const LiteralSymbolExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Static>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

        auto GetName() const -> const SymbolName&;

    private:
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_Name;
    };
}
