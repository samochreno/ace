#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    ConditionalJumpStmtBoundNode::ConditionalJumpStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& condition,
        LabelSymbol* const labelSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Condition{ condition },
        m_LabelSymbol{ labelSymbol }
    {
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

    auto ConditionalJumpStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ConditionalJumpStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedCondition = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (checkedCondition == m_Condition)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const ConditionalJumpStmtBoundNode>(
                GetSrcLocation(),
                checkedCondition,
                m_LabelSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto ConditionalJumpStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto ConditionalJumpStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ConditionalJumpStmtBoundNode>
    {
        const auto loweredCondition = m_Condition->CreateLoweredExpr({});

        if (loweredCondition == m_Condition)
        {
            return shared_from_this();
        }

        return std::make_shared<const ConditionalJumpStmtBoundNode>(
            GetSrcLocation(),
            loweredCondition,
            m_LabelSymbol
        )->CreateLowered({});
    }

    auto ConditionalJumpStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto ConditionalJumpStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        auto blockBuilder = std::make_unique<BlockBuilder>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        const auto conditionEmitResult = m_Condition->Emit(emitter);

        auto* const boolType = GetCompilation()->GetNatives().Bool.GetIRType(
            emitter.GetContext()
        );

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
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

    auto ConditionalJumpStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return std::vector
        {
            CFANode{ CFANodeKind::ConditionalJump, m_LabelSymbol }
        };
    }
}
