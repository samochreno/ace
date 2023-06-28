#include "Nodes/Stmts/BlockStmtNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Nodes/Stmts/StmtNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Block.hpp"

namespace Ace
{
    BlockStmtNode::BlockStmtNode(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::vector<std::shared_ptr<const IStmtNode>>& t_stmts
    ) : m_SelfScope{ t_selfScope },
        m_Stmts{ t_stmts }
    {
    }

    auto BlockStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto BlockStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const BlockStmtNode>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const IStmtNode>> clonedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(clonedStmts),
            [&](const std::shared_ptr<const IStmtNode>& t_stmt)
            {
                return t_stmt->CloneInScopeStmt(selfScope);
            }
        );

        return std::make_shared<const BlockStmtNode>(
            selfScope,
            clonedStmts
        );
    }

    auto BlockStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto BlockStmtNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Block>>
    {
        ACE_TRY(boundStmts, TransformExpectedVector(m_Stmts,
        [](const std::shared_ptr<const IStmtNode>& t_stmt)
        {
            return t_stmt->CreateBoundStmt();
        }));

        return std::make_shared<const BoundNode::Stmt::Block>(
            m_SelfScope,
            boundStmts
        );
    }

    auto BlockStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
