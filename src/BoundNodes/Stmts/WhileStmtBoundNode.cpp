#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdent.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "CFA.hpp"

namespace Ace
{
    WhileStmtBoundNode::WhileStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprBoundNode>& condition,
        const std::shared_ptr<const BlockStmtBoundNode>& body
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition },
        m_Body{ body }
    {
    }

    auto WhileStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto WhileStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto WhileStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto WhileStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const WhileStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedCondition = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        const auto checkedBody = diagnostics.Collect(m_Body->CreateTypeChecked({
            context.ParentFunctionTypeSymbol
        }));

        if (
            (checkedCondition == m_Condition) &&
            (checkedBody == m_Body)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const WhileStmtBoundNode>(
                GetSrcLocation(),
                m_Scope,
                checkedCondition,
                checkedBody
            ),
            diagnostics,
        };
    }

    auto WhileStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto WhileStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtBoundNode>
    {
        // From:
        // while condition {
        //     body;
        // }
        // 
        // To:
        // goto continue;
        // begin:
        // body;
        // continue:
        // gotoif condition begin;

        const Ident beginLabelName
        {
            GetSrcLocation().CreateFirst(),
            SpecialIdent::CreateAnonymous(),
        };
        auto beginLabelSymbolOwned = std::make_unique<LabelSymbol>(
            m_Scope,
            beginLabelName
        );

        auto* const beginLabelSymbol = dynamic_cast<LabelSymbol*>(
            m_Scope->DefineSymbol(std::move(beginLabelSymbolOwned)).Unwrap()
        );

        const Ident continueLabelName
        {
            GetSrcLocation().CreateLast(),
            SpecialIdent::CreateAnonymous(),
        };
        auto continueLabelSymbolOwned = std::make_unique<LabelSymbol>(
            m_Scope,
            continueLabelName
        );

        auto* const continueLabelSymbol = dynamic_cast<LabelSymbol*>(
            m_Scope->DefineSymbol(std::move(continueLabelSymbolOwned)).Unwrap()
        );

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        stmts.push_back(std::make_shared<const NormalJumpStmtBoundNode>(
            GetSrcLocation().CreateFirst(),
            m_Scope,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            beginLabelSymbol->GetName().SrcLocation,
            beginLabelSymbol
        ));

        stmts.push_back(m_Body);

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            continueLabelSymbol->GetName().SrcLocation,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const ConditionalJumpStmtBoundNode>(
            m_Condition->GetSrcLocation(),
            m_Condition,
            beginLabelSymbol
        ));

        return std::make_shared<const GroupStmtBoundNode>(
            GetSrcLocation(),
            m_Scope,
            stmts
        )->CreateLowered({});
    }

    auto WhileStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto WhileStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto WhileStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        ACE_UNREACHABLE();
    }
}
