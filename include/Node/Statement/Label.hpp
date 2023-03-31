#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Statement/Base.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class Label :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Label>,
        public virtual Node::IBindable<BoundNode::Statement::Label>,
        public virtual Node::ISymbolCreatable
    {
    public:
        Label(
            Scope* const t_scope,
            const std::string& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~Label() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Label> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Label>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

        auto GetSymbolScope() const -> Scope* { return m_Scope; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::Label; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto GetName() const -> const std::string& { return m_Name; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
    };
}
