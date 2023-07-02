#include "Nodes/Stmts/Assignments/CompoundAssignmentStmtNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace
{
    CompoundAssignmentStmtNode::CompoundAssignmentStmtNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr,
        const TokenKind& t_operator
    ) : m_Scope{ t_scope },
        m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_Operator{ t_operator }
    {
    }

    auto CompoundAssignmentStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto CompoundAssignmentStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto CompoundAssignmentStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const CompoundAssignmentStmtNode>
    {
        return std::make_shared<const CompoundAssignmentStmtNode>(
            m_Scope,
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope),
            m_Operator
        );
    }

    auto CompoundAssignmentStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto CompoundAssignmentStmtNode::CreateBound() const -> Expected<std::shared_ptr<const CompoundAssignmentStmtBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto operatorNameIt =
            SpecialIdentifier::Operator::BinaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(
            operatorNameIt != end(SpecialIdentifier::Operator::BinaryNameMap)
        );

        auto operatorFullName =
            lhsTypeSymbol->GetWithoutReference()->CreateFullyQualifiedName();
        operatorFullName.Sections.emplace_back(operatorNameIt->second);

        ACE_TRY(operatorSymbol, m_Scope->ResolveStaticSymbol<FunctionSymbol>(
            operatorFullName,
            Scope::CreateArgTypes({ lhsTypeSymbol, rhsTypeSymbol })
        ));

        return std::make_shared<const CompoundAssignmentStmtBoundNode>(
            boundLHSExpr,
            boundRHSExpr,
            operatorSymbol
        );
    }

    auto CompoundAssignmentStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
