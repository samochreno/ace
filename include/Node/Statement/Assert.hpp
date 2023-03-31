#pragma once

#include <memory>
#include <vector>

#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Assert.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class Assert :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Assert>,
        public virtual Node::IBindable<BoundNode::Statement::Assert>
    {
    public:
        Assert(
            Scope* const t_scope,
            const std::shared_ptr<const Node::Expression::IBase>& t_condition
        ) : m_Scope{ t_scope },
            m_Condition{ t_condition }
        {
        }
        virtual ~Assert() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Assert> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Assert>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
        std::shared_ptr<const Node::Expression::IBase> m_Condition{};
    };
}
