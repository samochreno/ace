#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockEndStmtBoundNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"

namespace Ace
{
    BlockStmtBoundNode::BlockStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope },
        m_Stmts{ stmts }
    {
    }

    auto BlockStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto BlockStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }
    
    auto BlockStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto BlockStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const BlockStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const BlockStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_SelfScope,
            m_Stmts
        );
    }

    auto BlockStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto BlockStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const BlockStmtBoundNode>>>
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

        return CreateChanged(std::make_shared<const BlockStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_SelfScope,
            cchCheckedContent.Value
        ));
    }

    auto BlockStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto BlockStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const BlockStmtBoundNode>>
    {
        const auto cchLoweredStmts = TransformCacheableVector(m_Stmts,
        [](const std::shared_ptr<const IStmtBoundNode>& stmt)
        {
            return stmt->GetOrCreateLoweredStmt({});
        });

        if (!cchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const BlockStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_SelfScope,
            cchLoweredStmts.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto BlockStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto BlockStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        const auto stmts = CreateExpanded();
        emitter.EmitFunctionBodyStmts(stmts);
    }

    auto BlockStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        auto stmts = m_Stmts;

        const auto blockEnd = std::make_shared<const BlockEndStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation().CreateLast(),
            m_SelfScope
        );
        stmts.push_back(blockEnd);

        return stmts;
    }
}
