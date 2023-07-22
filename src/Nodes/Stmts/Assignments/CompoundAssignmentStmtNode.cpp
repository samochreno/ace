#include "Nodes/Stmts/Assignments/CompoundAssignmentStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    CompoundAssignmentStmtNode::CompoundAssignmentStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr,
        const Op& op
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_Op{ op }
    {
    }

    auto CompoundAssignmentStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const CompoundAssignmentStmtNode>
    {
        return std::make_shared<const CompoundAssignmentStmtNode>(
            m_SrcLocation,
            m_Scope,
            m_LHSExpr->CloneInScopeExpr(scope),
            m_RHSExpr->CloneInScopeExpr(scope),
            m_Op
        );
    }

    auto CompoundAssignmentStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto CompoundAssignmentStmtNode::CreateBound() const -> Expected<std::shared_ptr<const CompoundAssignmentStmtBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto& opNameMap =
            SpecialIdent::Op::BinaryNameMap;

        const auto opNameIt = opNameMap.find(m_Op.TokenKind);
        ACE_TRY_ASSERT(opNameIt != end(opNameMap));

        auto opFullName = lhsTypeSymbol->GetWithoutRef()->CreateFullyQualifiedName(
            m_Op.SrcLocation
        );
        opFullName.Sections.emplace_back(Ident{
            m_Op.SrcLocation,
            opNameIt->second,
        });

        ACE_TRY(opSymbol, m_Scope->ResolveStaticSymbol<FunctionSymbol>(
            opFullName,
            Scope::CreateArgTypes({ lhsTypeSymbol, rhsTypeSymbol })
        ));

        return std::make_shared<const CompoundAssignmentStmtBoundNode>(
            GetSrcLocation(),
            boundLHSExpr,
            boundRHSExpr,
            opSymbol
        );
    }

    auto CompoundAssignmentStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
