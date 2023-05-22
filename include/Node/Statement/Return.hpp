#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Return.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class Return :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Return>,
        public virtual Node::IBindable<BoundNode::Statement::Return>
    {
    public:
        Return(
            const std::shared_ptr<Scope>& t_scope,
            const std::optional<std::shared_ptr<const Node::Expression::IBase>>& t_optExpression
        ) : m_Scope{ t_scope },
            m_OptExpression{ t_optExpression }
        {
        }
        virtual ~Return() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Return> final;
        auto CloneInScopeStatement(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Return>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const Node::Expression::IBase>> m_OptExpression{};
    };
}
