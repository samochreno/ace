#pragma once

#include <memory>
#include <vector>

#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Assignment/Compound.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement::Assignment
{
    class Compound :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Assignment::Compound>,
        public virtual Node::IBindable<BoundNode::Statement::Assignment::Compound>
    {
    public:
        Compound(
            Scope* const t_scope,
            const std::shared_ptr<const Node::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const Node::Expression::IBase>& t_rhsExpression,
            const Token::Kind::Set& t_operator
        ) : m_Scope{ t_scope },
            m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression },
            m_Operator{ t_operator }
        {
        }
        virtual ~Compound() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Assignment::Compound> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Assignment::Compound>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
        std::shared_ptr<const Node::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const Node::Expression::IBase> m_RHSExpression{};
        Token::Kind::Set m_Operator{};
    };
}
