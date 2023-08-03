#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/VarRefs/StaticVarRefExprBoundNode.hpp"
#include "Name.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class SymbolLiteralExprNode :
        public virtual IExprNode,
        public virtual ICloneableInScopeNode<SymbolLiteralExprNode>, 
        public virtual IBindableNode<StaticVarRefExprBoundNode>
    {
    public:
        SymbolLiteralExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& name
        );
        virtual ~SymbolLiteralExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const SymbolLiteralExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> std::shared_ptr<const StaticVarRefExprBoundNode> final;
        auto CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode> final;

        auto GetName() const -> const SymbolName&;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_Name;
    };
}
