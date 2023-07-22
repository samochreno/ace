#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Diagnostic.hpp"
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
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const Ident& name
        );
        virtual ~LabelStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const LabelStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const LabelStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope>;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto GetName() const -> const Ident&;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
    };
}
