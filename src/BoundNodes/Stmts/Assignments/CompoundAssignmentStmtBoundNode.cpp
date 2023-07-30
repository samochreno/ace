#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "BoundNodes/Exprs/RefExprBoundNode.hpp"
#include "BoundNodes/Exprs/DerefExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarRefs/StaticVarRefExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarRefs/InstanceVarRefExprBoundNode.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    CompoundAssignmentStmtBoundNode::CompoundAssignmentStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto CompoundAssignmentStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CompoundAssignmentStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto CompoundAssignmentStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
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
            GetSrcLocation(),
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

    static auto CreateStaticVarRefExprLoweredStmts(
        const SrcLocation& srcLocation,
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
            srcLocation,
            lhsExpr,
            rhsExpr,
            opSymbol
        );

        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            srcLocation,
            lhsExpr,
            userBinaryExpr
        );
        stmts.push_back(assignmentStmt);

        return stmts;
    }

    struct TempRefExprAndStmts
    {
        std::shared_ptr<const IExprBoundNode> Expr{};
        std::vector<std::shared_ptr<const IStmtBoundNode>> Stmts{};
    };

    static auto CreateLValueTempRefExprAndStmts(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TempRefExprAndStmts 
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        if (expr->GetTypeInfo().Symbol->IsRef())
        {
            return
            {
                expr,
                stmts,
            };
        }

        return
        {
            std::make_shared<const RefExprBoundNode>(
                expr->GetSrcLocation(),
                expr
            ),
            stmts,
        };
    }

    static auto CreateRValueTempRefExprAndStmts(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TempRefExprAndStmts
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        ACE_ASSERT(!expr->GetTypeInfo().Symbol->IsRef());

        const Ident tempVarName
        {
            expr->GetSrcLocation(),
            SpecialIdent::CreateAnonymous()
        };
        auto tempVarSymbolOwned = std::make_unique<LocalVarSymbol>(
            scope,
            tempVarName,
            expr->GetTypeInfo().Symbol
        );

        auto* const tempVarSymbol = dynamic_cast<LocalVarSymbol*>(
            scope->DefineSymbol(std::move(tempVarSymbolOwned)).Unwrap()
        );
        ACE_ASSERT(tempVarSymbol);

        const auto tempVarStmt = std::make_shared<const VarStmtBoundNode>(
            expr->GetSrcLocation(),
            tempVarSymbol,
            expr
        );
        stmts.push_back(tempVarStmt);

        const auto tempVarRefExpr = std::make_shared<const StaticVarRefExprBoundNode>(
            expr->GetSrcLocation(),
            scope,
            tempVarSymbol
        );

        return
        {
            std::make_shared<const RefExprBoundNode>(
                expr->GetSrcLocation(),
                tempVarRefExpr
            ),
            stmts,
        };
    }

    static auto CreateTempRefExprAndStmts(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TempRefExprAndStmts 
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        switch (expr->GetTypeInfo().ValueKind)
        {
            case ValueKind::L:
            {
                return CreateLValueTempRefExprAndStmts(expr, scope);
            }

            case ValueKind::R:
            {
                return CreateRValueTempRefExprAndStmts(expr, scope);
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto CreateInstanceVarRefExprLoweredStmts(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const InstanceVarRefExprBoundNode* const lhsExpr,
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
        //     temp_ref: &auto = lhs;
        //     temp_ref.var = temp_ref.field + rhs;
        // }
        // 
        // If lhs is R-value:
        // {
        //     temp: auto = lhs;
        //     temp_ref: &auto = temp;
        //     temp_ref.var = temp_ref.field + rhs;
        // }

        const auto tempRefExprAndStmts = CreateTempRefExprAndStmts(
            lhsExpr->GetExpr(),
            scope
        );

        const auto& tempRefExpr = tempRefExprAndStmts.Expr;
        stmts.insert(
            stmts.end(),
            begin(tempRefExprAndStmts.Stmts),
            end  (tempRefExprAndStmts.Stmts)
        );

        const Ident tempRefVarName
        {
            tempRefExpr->GetSrcLocation(),
            SpecialIdent::CreateAnonymous()
        };
        auto tempRefVarSymbolOwned = std::make_unique<LocalVarSymbol>(
            scope,
            tempRefVarName,
            tempRefExpr->GetTypeInfo().Symbol
        );

        auto* const tempRefVarSymbol = dynamic_cast<LocalVarSymbol*>(
            scope->DefineSymbol(std::move(tempRefVarSymbolOwned)).Unwrap()
        );
        ACE_ASSERT(tempRefVarSymbol);

        const auto tempRefVarStmt = std::make_shared<const VarStmtBoundNode>(
            tempRefExpr->GetSrcLocation(),
            tempRefVarSymbol,
            tempRefExpr
        );
        stmts.push_back(tempRefVarStmt);

        const auto tempRefVarRefExpr = std::make_shared<const StaticVarRefExprBoundNode>(
            tempRefExpr->GetSrcLocation(),
            scope,
            tempRefVarSymbol
        );

        const auto tempRefVarFieldRefExpr = std::make_shared<const InstanceVarRefExprBoundNode>(
            lhsExpr->GetSrcLocation(),
            tempRefVarRefExpr,
            lhsExpr->GetVarSymbol()
        );

        const auto userBinaryExpr = std::make_shared<const UserBinaryExprBoundNode>(
            rhsExpr->GetSrcLocation(),
            tempRefVarFieldRefExpr,
            rhsExpr,
            opSymbol
        );

        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            srcLocation,
            tempRefVarFieldRefExpr,
            userBinaryExpr
        );
        stmts.push_back(assignmentStmt);

        return stmts;
    }

    static auto IsStaticVarRefExpr(
        const IExprBoundNode* expr
    ) -> bool
    {
        while (const auto* const derefExpr = dynamic_cast<const DerefExprBoundNode*>(expr))
        {
            expr = derefExpr->GetExpr().get();
        }

        return
            dynamic_cast<const StaticVarRefExprBoundNode*>(expr) !=
            nullptr;
    }

    auto CompoundAssignmentStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>>
    {
        const auto stmts = [&]() -> std::vector<std::shared_ptr<const IStmtBoundNode>>
        {
            if (IsStaticVarRefExpr(m_LHSExpr.get()))
            {
                return CreateStaticVarRefExprLoweredStmts(
                    GetSrcLocation(),
                    m_LHSExpr,
                    m_RHSExpr,
                    m_OpSymbol
                );
            }
            
            if (const auto* const instanceVarRefExpr = dynamic_cast<const InstanceVarRefExprBoundNode*>(m_LHSExpr.get()))
            {
                return CreateInstanceVarRefExprLoweredStmts(
                    GetSrcLocation(),
                    GetScope(),
                    instanceVarRefExpr,
                    m_RHSExpr,
                    m_OpSymbol
                );
            }

            ACE_UNREACHABLE();
        }();

        return CreateChanged(std::make_shared<const GroupStmtBoundNode>(
            GetSrcLocation(),
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
