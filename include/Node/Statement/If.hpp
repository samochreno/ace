#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "Node/Statement/Block.hpp"
#include "BoundNode/Statement/If.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class If :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::If>,
        public virtual Node::IBindable<BoundNode::Statement::If>
    {
    public:
        If(
            Scope* const t_scope,
            const std::vector<std::shared_ptr<const Node::Expression::IBase>>& t_conditions,
            const std::vector<std::shared_ptr<const Node::Statement::Block>>& t_bodies
        ) : m_Scope{ t_scope },
            m_Conditions{ t_conditions },
            m_Bodies{ t_bodies }

        {
        }
        virtual ~If() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::If> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::If>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
        std::vector<std::shared_ptr<const Node::Expression::IBase>> m_Conditions{};
        std::vector<std::shared_ptr<const Node::Statement::Block>> m_Bodies{};
    };
}
