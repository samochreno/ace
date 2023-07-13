#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/VarReferences/InstanceVarReferenceExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class MemberAccessExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<MemberAccessExprNode>,
        public virtual IBindableNode<InstanceVarReferenceExprBoundNode>
    {
    public:
        MemberAccessExprNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<const IExprNode>& t_expr,
            const SymbolNameSection& t_name
        );
        virtual ~MemberAccessExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope>;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const MemberAccessExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const InstanceVarReferenceExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

        auto GetExpr() const -> const IExprNode*;
        auto GetName() const -> const SymbolNameSection&;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
        SymbolNameSection m_Name{};
    };
}
