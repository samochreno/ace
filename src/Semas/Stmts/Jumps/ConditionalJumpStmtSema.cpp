#include "Semas/Stmts/Jumps/ConditionalJumpStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    ConditionalJumpStmtSema::ConditionalJumpStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& condition,
        LabelSymbol* const labelSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Condition{ condition },
        m_LabelSymbol{ labelSymbol }
    {
    }

    auto ConditionalJumpStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("ConditionalJumpStmtSema", [&]()
        {
            logger.Log("m_Condition", m_Condition);
            logger.Log("m_LabelSymbol", m_LabelSymbol);
        });
    }

    auto ConditionalJumpStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ConditionalJumpStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto ConditionalJumpStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ConditionalJumpStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedCondition = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(m_Condition, typeInfo)
        );

        if (checkedCondition == m_Condition)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const ConditionalJumpStmtSema>(
                GetSrcLocation(),
                checkedCondition,
                m_LabelSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto ConditionalJumpStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto ConditionalJumpStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ConditionalJumpStmtSema>
    {
        const auto loweredCondition = m_Condition->CreateLoweredExpr({});

        if (loweredCondition == m_Condition)
        {
            return shared_from_this();
        }

        return std::make_shared<const ConditionalJumpStmtSema>(
            GetSrcLocation(),
            loweredCondition,
            m_LabelSymbol
        )->CreateLowered({});
    }

    auto ConditionalJumpStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto ConditionalJumpStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Condition);
    }

    auto ConditionalJumpStmtSema::Emit(Emitter& emitter) const -> void
    {
        auto block = std::make_unique<EmittingBlock>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        const auto conditionEmitResult = m_Condition->Emit(emitter);

        auto* const boolType =
            emitter.GetType(GetCompilation()->GetNatives().Bool.GetSymbol());

        auto* const loadInst = emitter.GetBlock().Builder.CreateLoad(
            boolType,
            conditionEmitResult.Value
        );

        emitter.EmitDropTmps(conditionEmitResult.Tmps);

        emitter.GetBlock().Builder.CreateCondBr(
            loadInst,
            emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol),
            block->Block
        );

        emitter.SetBlock(std::move(block));
    }

    auto ConditionalJumpStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return std::vector
        {
            ControlFlowNode{ ControlFlowKind::ConditionalJump, m_LabelSymbol }
        };
    }
}
