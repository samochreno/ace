#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Stmt/Base.hpp"
#include "BoundNode/Stmt/Label.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Stmt
{
    class Label :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Label>,
        public virtual Node::IBindable<BoundNode::Stmt::Label>,
        public virtual Node::ISymbolCreatable
    {
    public:
        Label(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~Label() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Label> final;
        auto CloneInScopeStmt(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Label>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final { return CreateBound(); }

        auto GetSymbolScope() const -> std::shared_ptr<Scope> { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::Label; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto GetName() const -> const std::string& { return m_Name; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
