#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Stmts/StmtNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class LabelStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<LabelStmtNode>,
        public virtual IBindableNode<LabelStmtBoundNode>,
        public virtual ISymbolCreatableNode
    {
    public:
        LabelStmtNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        );
        virtual ~LabelStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const LabelStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const LabelStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope>;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto GetName() const -> const std::string&;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
