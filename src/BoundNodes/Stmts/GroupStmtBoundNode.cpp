#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    GroupStmtBoundNode::GroupStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Stmts{ stmts }
    {
    }

    auto GroupStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto GroupStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto GroupStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto GroupStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto GroupStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const GroupStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const GroupStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            m_Stmts
        );
    }

    auto GroupStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto GroupStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const GroupStmtBoundNode>>>
    {
        ACE_TRY(cchCheckedContent, TransformExpectedCacheableVector(m_Stmts,
        [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
        {
            return stmt->GetOrCreateTypeCheckedStmt({
                context.ParentFunctionTypeSymbol
            });
        }));

        if (!cchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            cchCheckedContent.Value
        ));
    }

    auto GroupStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto GroupStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto cchLoweredStmts = TransformCacheableVector(m_Stmts,
        [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
        {
            return stmt->GetOrCreateLoweredStmt({});
        });

        if (!cchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            cchLoweredStmts.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto GroupStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto GroupStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
    
    auto GroupStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        return m_Stmts;
    }
}
