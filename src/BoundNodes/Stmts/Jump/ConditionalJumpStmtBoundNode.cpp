#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ConditionalJumpStmtBoundNode::ConditionalJumpStmtBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_condition,
        LabelSymbol* const t_labelSymbol
    ) : m_Condition{ t_condition },
        m_LabelSymbol{ t_labelSymbol }
    {
    }

    auto ConditionalJumpStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto ConditionalJumpStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (!mchConvertedAndCheckedCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ConditionalJumpStmtBoundNode>(
            mchConvertedAndCheckedCondition.Value,
            m_LabelSymbol
        ));
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>>
    {
        const auto mchLoweredCondition =
            m_Condition->GetOrCreateLoweredExpr({});

        if (!mchLoweredCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ConditionalJumpStmtBoundNode>(
            mchLoweredCondition.Value,
            m_LabelSymbol
        )->GetOrCreateLowered(t_context).Value);
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto ConditionalJumpStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        auto blockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            t_emitter.GetFunction()
        );

        const auto conditionEmitResult = m_Condition->Emit(t_emitter);

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            GetCompilation()->Natives->Bool.GetIRType(),
            conditionEmitResult.Value
        );

        t_emitter.EmitDropTemporaries(conditionEmitResult.Temporaries);

        t_emitter.GetBlockBuilder().Builder.CreateCondBr(
            loadInst,
            t_emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol),
            blockBuilder->Block
        );

        t_emitter.SetBlockBuilder(std::move(blockBuilder));
    }

    auto ConditionalJumpStmtBoundNode::GetLabelSymbol() const -> LabelSymbol*
    {
        return m_LabelSymbol;
    }
}
