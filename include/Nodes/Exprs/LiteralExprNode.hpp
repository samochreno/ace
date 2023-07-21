#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/LiteralExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class LiteralExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<LiteralExprNode>, 
        public virtual IBindableNode<LiteralExprBoundNode>
    {
    public:
        LiteralExprNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const LiteralKind kind,
            const std::string& string
        );
        virtual ~LiteralExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const LiteralExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const LiteralExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
