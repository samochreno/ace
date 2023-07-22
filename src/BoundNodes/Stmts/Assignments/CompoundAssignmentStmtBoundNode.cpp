#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
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
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) : m_SourceLocation{ sourceLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto CompoundAssignmentStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const CompoundAssignmentStmtBoundNode>>>
    {
        const auto argTypeInfos = m_OpSymbol->CollectArgTypeInfos();
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
            GetSourceLocation(),
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value,
            m_OpSymbol
        ));
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    static auto CreateStaticVarReferenceExprLoweredStmts(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        // From:
        // lhs += rhs;
        // 
        // To:
        // lhs = lhs + rhs;

        const auto userBinaryExpr = std::make_shared<const UserBinaryExprBoundNode>(
            sourceLocation,
            lhsExpr,
            rhsExpr,
            opSymbol
        );

        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            sourceLocation,
            lhsExpr,
            userBinaryExpr
        );
        stmts.push_back(assignmentStmt);

        return stmts;
    }

    struct TmpRefExprAndStmts
    {
        std::shared_ptr<const IExprBoundNode> Expr{};
        std::vector<std::shared_ptr<const IStmtBoundNode>> Stmts{};
    };

    static auto CreateLValueTmpRefExprAndStmts(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TmpRefExprAndStmts 
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        if (expr->GetTypeInfo().Symbol->IsReference())
        {
            return
            {
                expr,
                stmts,
            };
        }

        return
        {
            std::make_shared<const ReferenceExprBoundNode>(
                expr->GetSourceLocation(),
                expr
            ),
            stmts,
        };
    }

    static auto CreateRValueTmpRefExprAndStmts(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TmpRefExprAndStmts
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        ACE_ASSERT(!expr->GetTypeInfo().Symbol->IsReference());

        const Identifier tmpVarName
        {
            expr->GetSourceLocation(),
            SpecialIdentifier::CreateAnonymous()
        };
        auto tmpVarSymbolOwned = std::make_unique<LocalVarSymbol>(
            scope,
            tmpVarName,
            expr->GetTypeInfo().Symbol
        );

        auto* const tmpVarSymbol = dynamic_cast<LocalVarSymbol*>(
            scope->DefineSymbol(std::move(tmpVarSymbolOwned)).Unwrap()
        );
        ACE_ASSERT(tmpVarSymbol);

        const auto tmpVarStmt = std::make_shared<const VarStmtBoundNode>(
            expr->GetSourceLocation(),
            tmpVarSymbol,
            expr
        );
        stmts.push_back(tmpVarStmt);

        const auto tmpVarReferenceExpr = std::make_shared<const StaticVarReferenceExprBoundNode>(
            expr->GetSourceLocation(),
            scope,
            tmpVarSymbol
        );

        return
        {
            std::make_shared<const ReferenceExprBoundNode>(
                expr->GetSourceLocation(),
                tmpVarReferenceExpr
            ),
            stmts,
        };
    }

    static auto CreateTmpRefExprAndStmts(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TmpRefExprAndStmts 
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        switch (expr->GetTypeInfo().ValueKind)
        {
            case ValueKind::L:
            {
                return CreateLValueTmpRefExprAndStmts(expr, scope);
            }

            case ValueKind::R:
            {
                return CreateRValueTmpRefExprAndStmts(expr, scope);
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto CreateInstanceVarReferenceExprLoweredStmts(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const InstanceVarReferenceExprBoundNode* const lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        // From:
        // lhs.var += rhs;
        // 
        // To:
        //
        // If lhs is L-value:
        // {
        //     tmp_ref: &auto = lhs;
        //     tmp_ref.var = tmp_ref.field + rhs;
        // }
        // 
        // If lhs is R-value:
        // {
        //     tmp: auto = lhs;
        //     tmp_ref: &auto = tmp;
        //     tmp_ref.var = tmp_ref.field + rhs;
        // }

        const auto tmpRefExprAndStmts = CreateTmpRefExprAndStmts(
            lhsExpr->GetExpr(),
            scope
        );

        const auto& tmpRefExpr = tmpRefExprAndStmts.Expr;
        stmts.insert(
            stmts.end(),
            begin(tmpRefExprAndStmts.Stmts),
            end  (tmpRefExprAndStmts.Stmts)
        );

        const Identifier tmpRefVarName
        {
            tmpRefExpr->GetSourceLocation(),
            SpecialIdentifier::CreateAnonymous()
        };
        auto tmpRefVarSymbolOwned = std::make_unique<LocalVarSymbol>(
            scope,
            tmpRefVarName,
            tmpRefExpr->GetTypeInfo().Symbol
        );

        auto* const tmpRefVarSymbol = dynamic_cast<LocalVarSymbol*>(
            scope->DefineSymbol(std::move(tmpRefVarSymbolOwned)).Unwrap()
        );
        ACE_ASSERT(tmpRefVarSymbol);

        const auto tmpRefVarStmt = std::make_shared<const VarStmtBoundNode>(
            tmpRefExpr->GetSourceLocation(),
            tmpRefVarSymbol,
            tmpRefExpr
        );
        stmts.push_back(tmpRefVarStmt);

        const auto tmpRefVarReferenceExpr = std::make_shared<const StaticVarReferenceExprBoundNode>(
            tmpRefExpr->GetSourceLocation(),
            scope,
            tmpRefVarSymbol
        );

        const auto tmpRefVarFieldReferenceExpr = std::make_shared<const InstanceVarReferenceExprBoundNode>(
            lhsExpr->GetSourceLocation(),
            tmpRefVarReferenceExpr,
            lhsExpr->GetVarSymbol()
        );

        const auto userBinaryExpr = std::make_shared<const UserBinaryExprBoundNode>(
            rhsExpr->GetSourceLocation(),
            tmpRefVarFieldReferenceExpr,
            rhsExpr,
            opSymbol
        );

        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            sourceLocation,
            tmpRefVarFieldReferenceExpr,
            userBinaryExpr
        );
        stmts.push_back(assignmentStmt);

        return stmts;
    }

    static auto IsStaticVarReferenceExpr(
        const IExprBoundNode* expr
    ) -> bool
    {
        while (const auto* const derefExpr = dynamic_cast<const DereferenceExprBoundNode*>(expr))
        {
            expr = derefExpr->GetExpr().get();
        }

        return
            dynamic_cast<const StaticVarReferenceExprBoundNode*>(expr) !=
            nullptr;
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto stmts = [&]() -> std::vector<std::shared_ptr<const IStmtBoundNode>>
        {
            if (IsStaticVarReferenceExpr(m_LHSExpr.get()))
            {
                return CreateStaticVarReferenceExprLoweredStmts(
                    GetSourceLocation(),
                    m_LHSExpr,
                    m_RHSExpr,
                    m_OpSymbol
                );
            }
            
            if (const auto* const instanceVarReferenceExpr = dynamic_cast<const InstanceVarReferenceExprBoundNode*>(m_LHSExpr.get()))
            {
                return CreateInstanceVarReferenceExprLoweredStmts(
                    GetSourceLocation(),
                    GetScope(),
                    instanceVarReferenceExpr,
                    m_RHSExpr,
                    m_OpSymbol
                );
            }

            ACE_UNREACHABLE();
        }();

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            GetSourceLocation(),
            GetScope(),
            stmts
        )->GetOrCreateLowered({}).Value);
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto CompoundAssignmentStmtBoundNode::Emit(
        Emitter& emitter
    ) const -> void
    {
        ACE_UNREACHABLE();
    }
}
