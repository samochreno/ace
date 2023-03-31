#pragma once

#include <memory>
#include <vector>

#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "Node/Statement/Block.hpp"
#include "BoundNode/Statement/While.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class While :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::While>,
        public virtual Node::IBindable<BoundNode::Statement::While>
    {
    public:
        While(
            Scope* const t_scope,
            const std::shared_ptr<const Node::Expression::IBase>& t_condition,
            const std::shared_ptr<const Node::Statement::Block>& t_body
        ) : m_Scope{ t_scope },
            m_Condition{ t_condition },
            m_Body{ t_body }
        {
        }
        virtual ~While() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::While> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::While>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
        std::shared_ptr<const Node::Expression::IBase> m_Condition{};
        std::shared_ptr<const Node::Statement::Block> m_Body{};
    };
}
