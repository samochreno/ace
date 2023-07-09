#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNodes/Exprs/ReferenceExprBoundNode.hpp"
#include "BoundNodes/Exprs/DereferenceExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarReferences/InstanceVarReferenceExprBoundNode.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace
{
    CompoundAssignmentStmtBoundNode::CompoundAssignmentStmtBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& t_rhsExpr,
        FunctionSymbol* const t_operatorSymbol
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_OperatorSymbol{ t_operatorSymbol }
    {
    }

    auto CompoundAssignmentStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto CompoundAssignmentStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const CompoundAssignmentStmtBoundNode>>>
    {
        const auto argTypeInfos = m_OperatorSymbol->CollectArgTypeInfos();
        ACE_ASSERT(argTypeInfos.size() == 2);

        ACE_TRY(mchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            TypeInfo{ argTypeInfos.at(0).Symbol, ValueKind::L }
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            TypeInfo{ argTypeInfos.at(1).Symbol, ValueKind::R }
        ));

        if (
            !mchConvertedAndCheckedLHSExpr.IsChanged &&
            !mchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const CompoundAssignmentStmtBoundNode>(
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value,
            m_OperatorSymbol
        ));
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        const bool isStaticVarReferenceExpr = [&]() -> bool
        {
            auto* nextExpr = m_LHSExpr.get();
            while (const auto* const expr = dynamic_cast<const DereferenceExprBoundNode*>(nextExpr))
            {
                nextExpr = expr->GetExpr().get();
            }

            return dynamic_cast<const StaticVarReferenceExprBoundNode*>(nextExpr) != nullptr;
        }();

        if (isStaticVarReferenceExpr)
        {
            // From:
            // lhs += rhs;
            // 
            // To:
            // lhs = lhs + rhs;

            const auto userBinaryExpr = std::make_shared<const UserBinaryExprBoundNode>(
                m_LHSExpr,
                m_RHSExpr,
                m_OperatorSymbol
            );

            const auto assignmentStmt = std::make_shared<const AssignmentStmtBoundNode>(
                m_LHSExpr,
                userBinaryExpr
            );
            stmts.push_back(assignmentStmt);
        }
        else if (const auto* const instanceVarReferenceExpr = dynamic_cast<const InstanceVarReferenceExprBoundNode*>(m_LHSExpr.get()))
        {
            // From:
            // lhs.variable += rhs;
            // 
            // To:
            //
            // If lhs is L-value:
            // {
            //     tmp_ref: &auto = lhs;
            //     tmp_ref.variable = tmp_ref.field + rhs;
            // }
            // 
            // If lhs is R-value:
            // {
            //     tmp: auto = lhs;
            //     tmp_ref: &auto = tmp;
            //     tmp_ref.variable = tmp_ref.field + rhs;
            // }

            std::vector<std::shared_ptr<const IStmtBoundNode>> blockStmts{};
            const std::shared_ptr<Scope> blockScope = GetScope()->GetOrCreateChild({});

            const auto tmpRefExpr = [&]() -> std::shared_ptr<const IExprBoundNode>
            {
                const auto expr = instanceVarReferenceExpr->GetExpr();
                
                switch (expr->GetTypeInfo().ValueKind)
                {
                    case ValueKind::L:
                    {
                        if (expr->GetTypeInfo().Symbol->IsReference())
                        {
                            return expr;
                        }

                        return std::make_shared<const ReferenceExprBoundNode>(
                            expr
                        );
                    }

                    case ValueKind::R:
                    {
                        ACE_ASSERT(!expr->GetTypeInfo().Symbol->IsReference());

                        auto tmpVarSymbolOwned = std::make_unique<LocalVarSymbol>(
                            blockScope,
                            SpecialIdentifier::CreateAnonymous(),
                            expr->GetTypeInfo().Symbol
                        );

                        auto* const tmpVarSymbol = dynamic_cast<LocalVarSymbol*>(
                            blockScope->DefineSymbol(std::move(tmpVarSymbolOwned)).Unwrap()
                        );
                        ACE_ASSERT(tmpVarSymbol);

                        const auto tmpVarStmt = std::make_shared<const VarStmtBoundNode>(
                            tmpVarSymbol,
                            expr
                        );
                        blockStmts.push_back(tmpVarStmt);

                        const auto tmpVarReferenceExpr = std::make_shared<const StaticVarReferenceExprBoundNode>(
                            blockScope,
                            tmpVarSymbol
                        );

                        return std::make_shared<const ReferenceExprBoundNode>(
                            tmpVarReferenceExpr
                        );
                    }
                }
            }();

            auto tmpRefVarSymbolOwned = std::make_unique<LocalVarSymbol>(
                blockScope,
                SpecialIdentifier::CreateAnonymous(),
                tmpRefExpr->GetTypeInfo().Symbol
            );

            auto* const tmpRefVarSymbol = dynamic_cast<LocalVarSymbol*>(
                blockScope->DefineSymbol(std::move(tmpRefVarSymbolOwned)).Unwrap()
            );
            ACE_ASSERT(tmpRefVarSymbol);

            const auto tmpRefVarStmt = std::make_shared<const VarStmtBoundNode>(
                tmpRefVarSymbol,
                tmpRefExpr
            );
            blockStmts.push_back(tmpRefVarStmt);

            const auto tmpRefVarReferenceExpr = std::make_shared<const StaticVarReferenceExprBoundNode>(
                blockScope,
                tmpRefVarSymbol
            );

            const auto tmpRefVarFieldReferenceExpr = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                tmpRefVarReferenceExpr,
                instanceVarReferenceExpr->GetVarSymbol()
            );

            const auto userBinaryExpr = std::make_shared<const UserBinaryExprBoundNode>(
                tmpRefVarFieldReferenceExpr,
                m_RHSExpr,
                m_OperatorSymbol
            );

            const auto assignmentStmt = std::make_shared<const AssignmentStmtBoundNode>(
                tmpRefVarFieldReferenceExpr,
                userBinaryExpr
            );
            blockStmts.push_back(assignmentStmt);

            const auto blockStmt = std::make_shared<const BlockStmtBoundNode>(
                blockScope,
                blockStmts
            );
            stmts.push_back(blockStmt);
        }
        else
        {
            ACE_UNREACHABLE();
        }

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            GetScope(),
            stmts
        )->GetOrCreateLowered({}).Value);
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto CompoundAssignmentStmtBoundNode::Emit(
        Emitter& t_emitter
    ) const -> void
    {
        ACE_UNREACHABLE();
    }
}
