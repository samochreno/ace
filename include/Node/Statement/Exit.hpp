#pragma once

#include <memory>
#include <vector>

#include "Node/Statement/Base.hpp"
#include "BoundNode/Statement/Exit.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class Exit :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Exit>,
        public virtual Node::IBindable<BoundNode::Statement::Exit>
    {
    public:
        Exit(Scope* const t_scope)
            : m_Scope{ t_scope }
        {
        }
        virtual ~Exit() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Exit> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Exit>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
    };
}
