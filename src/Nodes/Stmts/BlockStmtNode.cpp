#include "Nodes/Stmts/BlockStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Stmts/StmtNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"

namespace Ace
{
    BlockStmtNode::BlockStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const std::vector<std::shared_ptr<const IStmtNode>>& stmts
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope },
        m_Stmts{ stmts }
    {
    }

    auto BlockStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const BlockStmtNode>
    {
        const auto selfScope = scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const IStmtNode>> clonedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(clonedStmts),
            [&](const std::shared_ptr<const IStmtNode>& stmt)
            {
                return stmt->CloneInScopeStmt(selfScope);
            }
        );

        return std::make_shared<const BlockStmtNode>(
            m_SrcLocation,
            selfScope,
            clonedStmts
        );
    }

    auto BlockStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto BlockStmtNode::CreateBound() const -> Expected<std::shared_ptr<const BlockStmtBoundNode>>
    {
        ACE_TRY(boundStmts, TransformExpectedVector(m_Stmts,
        [](const std::shared_ptr<const IStmtNode>& stmt)
        {
            return stmt->CreateBoundStmt();
        }));

        return std::make_shared<const BlockStmtBoundNode>(
            GetSrcLocation(),
            m_SelfScope,
            boundStmts
        );
    }

    auto BlockStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
