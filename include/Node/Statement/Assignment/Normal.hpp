#pragma once

#include <memory>
#include <vector>

#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Assignment/Normal.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement::Assignment
{
    class Normal :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Assignment::Normal>,
        public virtual Node::IBindable<BoundNode::Statement::Assignment::Normal>
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const Node::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const Node::Expression::IBase>& t_rhsExpression
        ) : m_Scope{ t_scope },
            m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Assignment::Normal> final;
        auto CloneInScopeStatement(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Assignment::Normal>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }
        
    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const Node::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const Node::Expression::IBase> m_RHSExpression{};
    };
}
