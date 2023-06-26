#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Node/Stmt/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Var.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace::Node::Stmt
{
    class Var :
        public virtual Node::Stmt::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Stmt::Var>,
        public virtual Node::IBindable<BoundNode::Stmt::Var>
    {
    public:
        Var(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::optional<std::shared_ptr<const Node::Expr::IBase>>& t_optAssignedExpr
        );
        virtual ~Var() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::Var> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Var>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

        auto GetName() const -> const std::string & final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::optional<std::shared_ptr<const Node::Expr::IBase>> m_OptAssignedExpr{};
    };
}
