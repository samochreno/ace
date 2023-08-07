#include "Nodes/Stmts/Assignments/CompoundAssignmentStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
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

    auto CompoundAssignmentStmtNode::CollectChildren() const -> std::vector<const INode*>
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

    static auto CreateFullyQualifiedOpName(
        const Op& op,
        ITypeSymbol* const typeSymbol
    ) -> SymbolName
    {
        const auto& name = SpecialIdent::Op::BinaryNameMap.at(op.TokenKind);

        auto fullyQualifiedName = typeSymbol->GetWithoutRef()->CreateFullyQualifiedName(
            op.SrcLocation
        );
        fullyQualifiedName.Sections.emplace_back(Ident{
            op.SrcLocation,
            name,
        });

        return fullyQualifiedName;
    }

    static auto ResolveOpSymbol(
        const std::shared_ptr<Scope>& scope,
        const Op& op,
        ITypeSymbol* const lhsTypeSymbol,
        ITypeSymbol* const rhsTypeSymbol
    ) -> Diagnosed<FunctionSymbol*>
    {
        DiagnosticBag diagnostics{};

        const auto expSymbol = scope->ResolveStaticSymbol<FunctionSymbol>(
            CreateFullyQualifiedOpName(op, lhsTypeSymbol),
            Scope::CreateArgTypes({ lhsTypeSymbol, rhsTypeSymbol })
        );
        if (!expSymbol)
        {
            diagnostics.Add(CreateUndefinedBinaryOpRefError(
                op,
                lhsTypeSymbol,
                rhsTypeSymbol
            ));

            auto* compilation = scope->GetCompilation();
            return Diagnosed
            {
                compilation->GetErrorSymbols().GetFunction(),
                diagnostics,
            };
        }

        const auto optSymbol = diagnostics.Collect(std::move(expSymbol));
        return Diagnosed{ optSymbol.value(), diagnostics };
    }

    auto CompoundAssignmentStmtNode::CreateBound() const -> Diagnosed<std::shared_ptr<const CompoundAssignmentStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto boundLHSExpr =
            diagnostics.Collect(m_LHSExpr->CreateBoundExpr());
        const auto boundRHSExpr =
            diagnostics.Collect(m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto opSymbol = diagnostics.Collect(ResolveOpSymbol(
            GetScope(),
            m_Op,
            lhsTypeSymbol,
            rhsTypeSymbol
        ));

        return Diagnosed
        {
            std::make_shared<const CompoundAssignmentStmtBoundNode>(
                GetSrcLocation(),
                boundLHSExpr,
                boundRHSExpr,
                opSymbol
            ),
            diagnostics,
        };
    }

    auto CompoundAssignmentStmtNode::CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
