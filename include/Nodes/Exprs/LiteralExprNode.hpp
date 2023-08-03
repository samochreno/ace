#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/LiteralExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"

namespace Ace
{
    class LiteralExprNode :
        public virtual IExprNode,
        public virtual ICloneableInScopeNode<LiteralExprNode>, 
        public virtual IBindableNode<LiteralExprBoundNode>
    {
    public:
        LiteralExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const LiteralKind kind,
            const std::string& string
        );
        virtual ~LiteralExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const LiteralExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> std::shared_ptr<const LiteralExprBoundNode> final;
        auto CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
