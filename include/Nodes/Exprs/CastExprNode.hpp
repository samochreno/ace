#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CastExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<CastExprNode>,
        public virtual IBindableNode<IExprBoundNode>
    {
    public:
        CastExprNode(
            const SourceLocation& t_sourceLocation,
            const TypeName& t_typeName,
            const std::shared_ptr<const IExprNode>& t_expr
        );
        virtual ~CastExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const CastExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
