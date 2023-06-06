#include "Node/Statement/Block.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Statement/Base.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Statement/Block.hpp"

namespace Ace::Node::Statement
{
    auto Block::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Statements);

        return children;
    }

    auto Block::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Block>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const Node::Statement::IBase>> clonedStatements{};
        std::transform(begin(m_Statements), end(m_Statements), back_inserter(clonedStatements),
        [&](const std::shared_ptr<const Node::Statement::IBase>& t_statement)
        {
            return t_statement->CloneInScopeStatement(selfScope);
        });

        return std::make_shared<const Node::Statement::Block>(
            selfScope,
            clonedStatements
        );
    }

    auto Block::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Block>>
    {
        ACE_TRY(boundStatements, TransformExpectedVector(m_Statements,
        [](const std::shared_ptr<const Node::Statement::IBase>& t_statement)
        {
            return t_statement->CreateBoundStatement();
        }));

        return std::make_shared<const BoundNode::Statement::Block>(
            m_SelfScope,
            boundStatements
            );
    }
}
