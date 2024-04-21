#include "Semas/Stmts/IfStmtSema.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/LogicalNegationExprSema.hpp"
#include "Semas/Stmts/LabelStmtSema.hpp"
#include "Semas/Stmts/Jumps/ConditionalJumpStmtSema.hpp"
#include "Semas/Stmts/Jumps/NormalJumpStmtSema.hpp"
#include "SemaLogger.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "AnonymousIdent.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    IfStmtSema::IfStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IExprSema>>& conditions,
        const std::vector<std::shared_ptr<const BlockStmtSema>>& blocks
    ) : m_Scope{ scope },
        m_Conditions{ conditions },
        m_Blocks{ blocks }
    {
    }

    auto IfStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("IfStmtSema", [&]()
        {
            logger.Log("m_Conditions", m_Conditions);
            logger.Log("m_Blocks", m_Blocks);
        });
    }

    auto IfStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto IfStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto IfStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IfStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };

        std::vector<std::shared_ptr<const IExprSema>> checkedConditions{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(checkedConditions),
            [&](const std::shared_ptr<const IExprSema>& condition)
            {
                return diagnostics.Collect(
                    CreateImplicitlyConvertedAndTypeChecked(condition, typeInfo)
                );
            }
        );

        std::vector<std::shared_ptr<const BlockStmtSema>> checkedBlocks{};
        std::transform(
            begin(m_Blocks),
            end  (m_Blocks),
            back_inserter(checkedBlocks),
            [&](const std::shared_ptr<const BlockStmtSema>& block)
            {
                return diagnostics.Collect(block->CreateTypeChecked({
                    context.ParentFunctionTypeSymbol
                }));
            }
        );

        if (
            (checkedConditions == m_Conditions) &&
            (checkedBlocks == m_Blocks)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const IfStmtSema>(
                GetSrcLocation(),
                GetScope(),
                checkedConditions,
                checkedBlocks
            ),
            std::move(diagnostics),
        };
    }

    auto IfStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto IfStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtSema>
    {
        // From:
        // if condition_0 {
        //     block_0;
        // } elif condition_1 {
        //     block_1;
        // } else {
        //     block_2;
        // }
        // 
        // To: 
        // gotoif !condition_0 label_0;
        // block_0;
        // goto label_2;
        // 
        // label_0:
        // gotoif !condition_1 label_1;
        // block_1;
        // goto label_2;
        // 
        // label_1:
        // block_2;
        // 
        // label_2:
        
        const bool hasElse = m_Blocks.size() > m_Conditions.size();

        const auto lastBlock = m_Blocks.back();

        std::vector<LabelSymbol*> labelSymbols{};

        const size_t labelCount = hasElse ?
            (m_Conditions.size() + 1) :
            (m_Conditions.size());

        for (size_t i = 0; i < labelCount; i++)
        {
            const auto labelSrcLocation = [&]()
            {
                if (i == (labelCount - 1))
                {
                    return lastBlock->GetSrcLocation().CreateFirst();
                }

                if (hasElse && (i == (labelCount - 2)))
                {
                    return lastBlock->GetSrcLocation().CreateLast();
                }

                return m_Conditions.at(i + 1)->GetSrcLocation();
            }();
            const Ident labelName{ labelSrcLocation, AnonymousIdent::Create() };
            auto labelSymbolOwned = std::make_unique<LabelSymbol>(
                GetScope(),
                labelName
            );

            auto* const labelSymbol = dynamic_cast<LabelSymbol*>(DiagnosticBag::CreateNoError().Collect(
                GetScope()->DeclareSymbol(std::move(labelSymbolOwned))
            ));

            labelSymbols.push_back(labelSymbol);
        }

        auto* const lastLabelSymbol = labelSymbols.back();

        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

        for (size_t i = 0; i < m_Conditions.size(); i++)
        {
            const bool isFirstBlock = i == 0;
            const bool isLastBlock  = i == (m_Blocks.size() - 1);

            if (!isFirstBlock)
            {
                auto* const labelSymbol = labelSymbols.at(i - 1);
                stmts.push_back(std::make_shared<const LabelStmtSema>(
                    labelSymbol->GetName().SrcLocation,
                    labelSymbol
                ));
            }

            const auto condition = std::make_shared<const LogicalNegationExprSema>(
                m_Conditions.at(i)->GetSrcLocation(),
                m_Conditions.at(i)
            );

            stmts.push_back(std::make_shared<const ConditionalJumpStmtSema>(
                condition->GetSrcLocation(),
                condition,
                labelSymbols.at(i)
            ));

            const auto block = m_Blocks.at(i);
            stmts.push_back(block);

            if (!isLastBlock)
            {
                stmts.push_back(std::make_shared<const NormalJumpStmtSema>(
                    block->GetSrcLocation().CreateLast(),
                    GetScope(),
                    lastLabelSymbol
                ));
            }
        }

        if (hasElse)
        {
            auto* const elseLabelSymbol = labelSymbols.rbegin()[1];

            stmts.push_back(std::make_shared<const LabelStmtSema>(
                elseLabelSymbol->GetName().SrcLocation,
                elseLabelSymbol
            ));

            stmts.push_back(lastBlock);
        }

        stmts.push_back(std::make_shared<const LabelStmtSema>(
            lastLabelSymbol->GetName().SrcLocation,
            lastLabelSymbol
        ));

        return std::make_shared<const GroupStmtSema>(
            GetSrcLocation(),
            m_Scope,
            stmts
        )->CreateLowered({});
    }

    auto IfStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto IfStmtSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto IfStmtSema::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto IfStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        ACE_UNREACHABLE();
    }
}
