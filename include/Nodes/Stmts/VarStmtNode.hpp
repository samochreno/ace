#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class VarStmtNode :
        public virtual IStmtNode,
        public virtual ITypedNode,
        public virtual ICloneableNode<VarStmtNode>,
        public virtual IBindableNode<VarStmtBoundNode>
    {
    public:
        VarStmtNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            const Identifier& t_name,
            const TypeName& t_typeName,
            const std::optional<std::shared_ptr<const IExprNode>>& t_optAssignedExpr
        );
        virtual ~VarStmtNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const VarStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const VarStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

        auto GetName() const -> const Identifier& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Identifier m_Name{};
        TypeName m_TypeName{};
        std::optional<std::shared_ptr<const IExprNode>> m_OptAssignedExpr{};
    };
}
