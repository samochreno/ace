#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdent.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    WhileStmtBoundNode::WhileStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprBoundNode>& condition,
        const std::shared_ptr<const BlockStmtBoundNode>& body
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition },
        m_Body{ body }
    {
    }

    auto WhileStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
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

    auto WhileStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const WhileStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const WhileStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            m_Condition,
            m_Body
        );
    }

    auto WhileStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto WhileStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const WhileStmtBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        ACE_TRY(mchCheckedBody, m_Body->GetOrCreateTypeChecked({
            context.ParentFunctionTypeSymbol
        }));

        if (
            !mchConvertedAndCheckedCondition.IsChanged &&
            !mchCheckedBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const WhileStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Scope,
            mchConvertedAndCheckedCondition.Value,
            mchCheckedBody.Value
        ));
    }

    auto WhileStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto WhileStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
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
            DiagnosticBag{},
            GetSrcLocation().CreateFirst(),
            m_Scope,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            DiagnosticBag{},
            beginLabelSymbol->GetName().SrcLocation,
            beginLabelSymbol
        ));

        stmts.push_back(m_Body);

        stmts.push_back(std::make_shared<const LabelStmtBoundNode>(
            DiagnosticBag{},
            continueLabelSymbol->GetName().SrcLocation,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const ConditionalJumpStmtBoundNode>(
            DiagnosticBag{},
            m_Condition->GetSrcLocation(),
            m_Condition,
            beginLabelSymbol
        ));

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Scope,
            stmts
        )->GetOrCreateLowered(context).Value);
    }

    auto WhileStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto WhileStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
