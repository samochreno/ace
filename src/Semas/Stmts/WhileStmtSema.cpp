#include "Semas/Stmts/WhileStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Stmts/LabelStmtSema.hpp"
#include "Semas/Stmts/Jumps/NormalJumpStmtSema.hpp"
#include "Semas/Stmts/Jumps/ConditionalJumpStmtSema.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "AnonymousIdent.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    WhileStmtSema::WhileStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprSema>& condition,
        const std::shared_ptr<const BlockStmtSema>& block
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition },
        m_Block{ block }
    {
    }

    auto WhileStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto WhileStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto WhileStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const WhileStmtSema>>
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

        const auto checkedBlock = diagnostics.Collect(
            m_Block->CreateTypeChecked({ context.ParentFunctionTypeSymbol })
        );

        if (
            (checkedCondition == m_Condition) &&
            (checkedBlock == m_Block)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const WhileStmtSema>(
                GetSrcLocation(),
                m_Scope,
                checkedCondition,
                checkedBlock
            ),
            std::move(diagnostics),
        };
    }

    auto WhileStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto WhileStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtSema>
    {
        // From:
        // while condition {
        //     block;
        // }
        // 
        // To:
        // goto continue;
        // begin:
        // block;
        // continue:
        // gotoif condition begin;

        const Ident beginLabelName
        {
            GetSrcLocation().CreateFirst(),
            AnonymousIdent::Create("begin"),
        };
        auto beginLabelSymbolOwned =
            std::make_unique<LabelSymbol>(m_Scope, beginLabelName);

        auto* const beginLabelSymbol = dynamic_cast<LabelSymbol*>(DiagnosticBag::CreateNoError().Collect(
            GetScope()->DeclareSymbol(std::move(beginLabelSymbolOwned))
        ));

        const Ident continueLabelName
        {
            GetSrcLocation().CreateLast(),
            AnonymousIdent::Create("continue"),
        };
        auto continueLabelSymbolOwned =
            std::make_unique<LabelSymbol>(m_Scope, continueLabelName);

        auto* const continueLabelSymbol = dynamic_cast<LabelSymbol*>(DiagnosticBag::CreateNoError().Collect(
            GetScope()->DeclareSymbol(std::move(continueLabelSymbolOwned))
        ));

        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

        stmts.push_back(std::make_shared<const NormalJumpStmtSema>(
            GetSrcLocation().CreateFirst(),
            m_Scope,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const LabelStmtSema>(
            beginLabelSymbol->GetName().SrcLocation,
            beginLabelSymbol
        ));

        stmts.push_back(m_Block);

        stmts.push_back(std::make_shared<const LabelStmtSema>(
            continueLabelSymbol->GetName().SrcLocation,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const ConditionalJumpStmtSema>(
            m_Condition->GetSrcLocation(),
            m_Condition,
            beginLabelSymbol
        ));

        return std::make_shared<const GroupStmtSema>(
            GetSrcLocation(),
            m_Scope,
            stmts
        )->CreateLowered({});
    }

    auto WhileStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto WhileStmtSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto WhileStmtSema::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto WhileStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        ACE_UNREACHABLE();
    }
}
