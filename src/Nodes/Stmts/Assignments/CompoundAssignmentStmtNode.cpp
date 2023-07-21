#include "Nodes/Stmts/Assignments/CompoundAssignmentStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Operator.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace
{
    CompoundAssignmentStmtNode::CompoundAssignmentStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr,
        const Operator& t_operator
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_Operator{ t_operator }
    {
    }

    auto CompoundAssignmentStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
            m_SourceLocation,
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

        const auto& operatorNameMap =
            SpecialIdentifier::Operator::BinaryNameMap;

        const auto operatorNameIt = operatorNameMap.find(m_Operator.TokenKind);
        ACE_TRY_ASSERT(operatorNameIt != end(operatorNameMap));

        auto operatorFullName = lhsTypeSymbol->GetWithoutReference()->CreateFullyQualifiedName(
            m_Operator.SourceLocation
        );
        operatorFullName.Sections.emplace_back(Identifier{
            m_Operator.SourceLocation,
            operatorNameIt->second,
        });

        ACE_TRY(operatorSymbol, m_Scope->ResolveStaticSymbol<FunctionSymbol>(
            operatorFullName,
            Scope::CreateArgTypes({ lhsTypeSymbol, rhsTypeSymbol })
        ));

        return std::make_shared<const CompoundAssignmentStmtBoundNode>(
            GetSourceLocation(),
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
