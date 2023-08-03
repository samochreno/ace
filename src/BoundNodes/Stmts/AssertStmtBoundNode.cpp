#include "BoundNodes/Stmts/AssertStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    AssertStmtBoundNode::AssertStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& condition
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Condition{ condition }
    {
    }

    auto AssertStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto AssertStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AssertStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto AssertStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto AssertStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const AssertStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const AssertStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Condition
        );
    }

    auto AssertStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto AssertStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const AssertStmtBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(cchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (!cchConvertedAndCheckedCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AssertStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedCondition.Value
        ));
    }

    auto AssertStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto AssertStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto cchLoweredCondition =
            m_Condition->GetOrCreateLoweredExpr({});

        const auto condition = std::make_shared<const LogicalNegationExprBoundNode>(
            DiagnosticBag{},
            cchLoweredCondition.Value->GetSrcLocation(),
            cchLoweredCondition.Value
        );

        const auto bodyScope = GetScope()->GetOrCreateChild({});

        const auto exitStmt = std::make_shared<const ExitStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            bodyScope
        );

        const auto bodyStmt = std::make_shared<const BlockStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            bodyScope,
            std::vector<std::shared_ptr<const IStmtBoundNode>>{ exitStmt }
        );

        return CreateChanged(std::make_shared<const IfStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            std::vector<std::shared_ptr<const IExprBoundNode>>{ condition },
            std::vector<std::shared_ptr<const BlockStmtBoundNode>>{ bodyStmt }
        )->GetOrCreateLowered({}).Value);
    };

    auto AssertStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto AssertStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
