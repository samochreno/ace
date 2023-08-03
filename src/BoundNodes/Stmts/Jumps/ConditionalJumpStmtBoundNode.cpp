#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ConditionalJumpStmtBoundNode::ConditionalJumpStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& condition,
        LabelSymbol* const labelSymbol
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Condition{ condition },
        m_LabelSymbol{ labelSymbol }
    {
    }

    auto ConditionalJumpStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto ConditionalJumpStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ConditionalJumpStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto ConditionalJumpStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto ConditionalJumpStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ConditionalJumpStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const ConditionalJumpStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Condition,
            GetLabelSymbol()
        );
    }

    auto ConditionalJumpStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const ConditionalJumpStmtBoundNode>>>
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

        return CreateChanged(std::make_shared<const ConditionalJumpStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedCondition.Value,
            m_LabelSymbol
        ));
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const ConditionalJumpStmtBoundNode>>
    {
        const auto cchLoweredCondition =
            m_Condition->GetOrCreateLoweredExpr({});

        if (!cchLoweredCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ConditionalJumpStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredCondition.Value,
            m_LabelSymbol
        )->GetOrCreateLowered(context).Value);
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto ConditionalJumpStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        auto blockBuilder = std::make_unique<BlockBuilder>(
            GetCompilation()->GetLLVMContext(),
            emitter.GetFunction()
        );

        const auto conditionEmitResult = m_Condition->Emit(emitter);

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            GetCompilation()->GetNatives()->Bool.GetIRType(),
            conditionEmitResult.Value
        );

        emitter.EmitDropTmps(conditionEmitResult.Tmps);

        emitter.GetBlockBuilder().Builder.CreateCondBr(
            loadInst,
            emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol),
            blockBuilder->Block
        );

        emitter.SetBlockBuilder(std::move(blockBuilder));
    }

    auto ConditionalJumpStmtBoundNode::GetLabelSymbol() const -> LabelSymbol*
    {
        return m_LabelSymbol;
    }
}
