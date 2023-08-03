#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/VarRefs/InstanceVarRefExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"

namespace Ace
{
    class MemberAccessExprNode :
        public virtual IExprNode,
        public virtual ICloneableInScopeNode<MemberAccessExprNode>,
        public virtual IBindableNode<InstanceVarRefExprBoundNode>
    {
    public:
        MemberAccessExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr,
            const SymbolNameSection& name
        );
        virtual ~MemberAccessExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope>;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const MemberAccessExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> std::shared_ptr<const InstanceVarRefExprBoundNode> final;
        auto CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode> final;

        auto GetExpr() const -> const IExprNode*;
        auto GetName() const -> const SymbolNameSection&;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
        SymbolNameSection m_Name{};
    };
}
