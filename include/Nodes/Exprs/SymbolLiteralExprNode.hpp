#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"
#include "Name.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class SymbolLiteralExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<SymbolLiteralExprNode>, 
        public virtual IBindableNode<StaticVarReferenceExprBoundNode>
    {
    public:
        SymbolLiteralExprNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& name
        );
        virtual ~SymbolLiteralExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const SymbolLiteralExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const StaticVarReferenceExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

        auto GetName() const -> const SymbolName&;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_Name;
    };
}
