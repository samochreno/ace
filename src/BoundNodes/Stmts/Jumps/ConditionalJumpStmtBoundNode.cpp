#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ConditionalJumpStmtBoundNode::ConditionalJumpStmtBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& condition,
        LabelSymbol* const labelSymbol
    ) : m_SourceLocation{ sourceLocation },
        m_Condition{ condition },
        m_LabelSymbol{ labelSymbol }
    {
    }

    auto ConditionalJumpStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const StmtTypeCheckingContext& context
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
            GetSourceLocation(),
            mchConvertedAndCheckedCondition.Value,
            m_LabelSymbol
        ));
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>>
    {
        const auto mchLoweredCondition =
            m_Condition->GetOrCreateLoweredExpr({});

        if (!mchLoweredCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ConditionalJumpStmtBoundNode>(
            GetSourceLocation(),
            mchLoweredCondition.Value,
            m_LabelSymbol
        )->GetOrCreateLowered(context).Value);
    }

    auto ConditionalJumpStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto ConditionalJumpStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        auto blockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            emitter.GetFunction()
        );

        const auto conditionEmitResult = m_Condition->Emit(emitter);

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            GetCompilation()->Natives->Bool.GetIRType(),
            conditionEmitResult.Value
        );

        emitter.EmitDropTemporaries(conditionEmitResult.Temporaries);

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
