#pragma once

#include <memory>
#include <vector>

#include "Node/Statement/Base.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    class Block :
        public virtual Node::Statement::IBase,
        public virtual Node::ICloneable<Node::Statement::Block>,
        public virtual Node::IBindable<BoundNode::Statement::Block>
    {
    public:
        Block(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<std::shared_ptr<const Node::Statement::IBase>>& t_statements
        ) : m_SelfScope{ t_selfScope },
            m_Statements{ t_statements }
        {
        }
        virtual ~Block() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_SelfScope->GetParent().value(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Block> final;
        auto CloneInScopeStatement(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Block>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::vector<std::shared_ptr<const Node::Statement::IBase>> m_Statements{};
    };
}
