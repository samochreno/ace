#include "Node/Stmt/Block.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Stmt/Base.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Block.hpp"

namespace Ace::Node::Stmt
{
    Block::Block(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::vector<std::shared_ptr<const Node::Stmt::IBase>>& t_stmts
    ) : m_SelfScope{ t_selfScope },
        m_Stmts{ t_stmts }
    {
    }

    auto Block::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Block::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto Block::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::Block>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const Node::Stmt::IBase>> clonedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(clonedStmts),
            [&](const std::shared_ptr<const Node::Stmt::IBase>& t_stmt)
            {
                return t_stmt->CloneInScopeStmt(selfScope);
            }
        );

        return std::make_shared<const Node::Stmt::Block>(
            selfScope,
            clonedStmts
        );
    }

    auto Block::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Block::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Block>>
    {
        ACE_TRY(boundStmts, TransformExpectedVector(m_Stmts,
        [](const std::shared_ptr<const Node::Stmt::IBase>& t_stmt)
        {
            return t_stmt->CreateBoundStmt();
        }));

        return std::make_shared<const BoundNode::Stmt::Block>(
            m_SelfScope,
            boundStmts
        );
    }

    auto Block::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
