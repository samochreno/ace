#include "Semas/Stmts/Assignments/CompoundAssignmentStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "SemaLogger.hpp"
#include "Semas/Exprs/RefExprSema.hpp"
#include "Semas/Exprs/DerefExprSema.hpp"
#include "Semas/Exprs/VarRefs/StaticVarRefExprSema.hpp"
#include "Semas/Exprs/VarRefs/FieldVarRefExprSema.hpp"
#include "Semas/Exprs/UserBinaryExprSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Semas/Stmts/ExprStmtSema.hpp"
#include "Semas/Stmts/Assignments/SimpleAssignmentStmtSema.hpp"
#include "Semas/Stmts/VarStmtSema.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "AnonymousIdent.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    CompoundAssignmentStmtSema::CompoundAssignmentStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& lhsExpr,
        const std::shared_ptr<const IExprSema>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto CompoundAssignmentStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("CompoundAssignmentStmtSema", [&]()
        {
            logger.Log("m_LHSExpr", m_LHSExpr);
            logger.Log("m_RHSExpr", m_RHSExpr);
            logger.Log("m_OpSymbol", m_OpSymbol);
        });
    }

    auto CompoundAssignmentStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CompoundAssignmentStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto CompoundAssignmentStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const CompoundAssignmentStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto convertedLHSExpr = m_LHSExpr;
        auto convertedRHSExpr = m_RHSExpr;
        if (!m_OpSymbol->IsError())
        {
            const auto argTypeInfos = m_OpSymbol->CollectAllArgTypeInfos();
            ACE_ASSERT(argTypeInfos.size() == 2);

            convertedLHSExpr = diagnostics.Collect(CreateImplicitlyConverted(
                convertedLHSExpr,
                TypeInfo{ argTypeInfos.at(0).Symbol, ValueKind::L }
            ));
            convertedRHSExpr = diagnostics.Collect(CreateImplicitlyConverted(
                convertedRHSExpr,
                TypeInfo{ argTypeInfos.at(1).Symbol, ValueKind::R }
            ));
        }

        const auto checkedLHSExpr = diagnostics.Collect(
            convertedLHSExpr->CreateTypeCheckedExpr({})
        );
        const auto checkedRHSExpr = diagnostics.Collect(
            convertedRHSExpr->CreateTypeCheckedExpr({})
        );
        
        if (
            (checkedLHSExpr == m_LHSExpr) &&
            (checkedRHSExpr == m_RHSExpr)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const CompoundAssignmentStmtSema>(
                GetSrcLocation(),
                checkedLHSExpr,
                checkedRHSExpr,
                m_OpSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto CompoundAssignmentStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    static auto CreateStaticVarRefExprLoweredStmts(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& assignmentLHSExpr,
        const std::shared_ptr<const IExprSema>& opLHSExpr,
        const std::shared_ptr<const IExprSema>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) -> std::vector<std::shared_ptr<const IStmtSema>>
    {
        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

        const auto loweredAssignmentLHSExpr = [&]() -> std::shared_ptr<const IExprSema>
        {
            if (!assignmentLHSExpr->GetTypeInfo().Symbol->IsRef())
            {
                return assignmentLHSExpr;
            }

            return std::make_shared<const DerefExprSema>(
                assignmentLHSExpr->GetSrcLocation(),
                assignmentLHSExpr
            );
        }();

        // From:
        // lhs += rhs;
        // 
        // To:
        // lhs = lhs + rhs;

        const auto userBinaryExpr = std::make_shared<const UserBinaryExprSema>(
            srcLocation,
            opLHSExpr,
            rhsExpr,
            opSymbol
        );

        const auto assignmentStmt = std::make_shared<const SimpleAssignmentStmtSema>(
            srcLocation,
            loweredAssignmentLHSExpr,
            userBinaryExpr
        );
        stmts.push_back(assignmentStmt);

        return stmts;
    }

    struct TempRefExprAndStmts
    {
        std::shared_ptr<const IExprSema> Expr{};
        std::vector<std::shared_ptr<const IStmtSema>> Stmts{};
    };

    static auto CreateLValueTempRefExprAndStmts(
        const std::shared_ptr<const IExprSema>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TempRefExprAndStmts 
    {
        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

        if (expr->GetTypeInfo().Symbol->IsRef())
        {
            return TempRefExprAndStmts{ expr, stmts };
        }

        return TempRefExprAndStmts
        {
            std::make_shared<const RefExprSema>(expr->GetSrcLocation(), expr),
            stmts,
        };
    }

    static auto CreateRValueTempRefExprAndStmts(
        const std::shared_ptr<const IExprSema>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TempRefExprAndStmts
    {
        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

        ACE_ASSERT(!expr->GetTypeInfo().Symbol->IsRef());

        auto* const exprType = dynamic_cast<ISizedTypeSymbol*>(
            expr->GetTypeInfo().Symbol
        );
        ACE_ASSERT(exprType);

        const Ident tmpVarName
        {
            expr->GetSrcLocation(),
            AnonymousIdent::Create("tmp")
        };
        auto tmpVarSymbolOwned = std::make_unique<LocalVarSymbol>(
            scope,
            tmpVarName,
            exprType
        );

        auto* const tmpVarSymbol = dynamic_cast<LocalVarSymbol*>(
            DiagnosticBag::CreateNoError().Collect(
                scope->DeclareSymbol(std::move(tmpVarSymbolOwned))
            )
        );
        ACE_ASSERT(tmpVarSymbol);

        const auto tmpVarStmt = std::make_shared<const VarStmtSema>(
            expr->GetSrcLocation(),
            tmpVarSymbol,
            expr
        );
        stmts.push_back(tmpVarStmt);

        const auto tmpVarRefExpr = std::make_shared<const StaticVarRefExprSema>(
            expr->GetSrcLocation(),
            scope,
            tmpVarSymbol
        );

        return
        {
            std::make_shared<const RefExprSema>(
                expr->GetSrcLocation(),
                tmpVarRefExpr
            ),
            stmts,
        };
    }

    static auto CreateTempRefExprAndStmts(
        const std::shared_ptr<const IExprSema>& expr,
        const std::shared_ptr<Scope>& scope
    ) -> TempRefExprAndStmts 
    {
        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

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

    static auto CreateFieldVarRefExprLoweredStmts(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const FieldVarRefExprSema* const lhsExpr,
        const std::shared_ptr<const IExprSema>& opLHSExprTemplate,
        const std::shared_ptr<const IExprSema>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) -> std::vector<std::shared_ptr<const IStmtSema>>
    {
        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

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

        const auto tmpRefExprAndStmts = CreateTempRefExprAndStmts(
            lhsExpr->GetExpr(),
            scope
        );

        const auto& tmpRefExpr = tmpRefExprAndStmts.Expr;
        stmts.insert(
            stmts.end(),
            begin(tmpRefExprAndStmts.Stmts),
            end  (tmpRefExprAndStmts.Stmts)
        );

        auto* const tmpRefType = dynamic_cast<ISizedTypeSymbol*>(
            tmpRefExpr->GetTypeInfo().Symbol
        );
        ACE_ASSERT(tmpRefType);

        const Ident tmpRefVarName
        {
            tmpRefExpr->GetSrcLocation(),
            AnonymousIdent::Create("tmp_ref")
        };
        auto tmpRefVarSymbolOwned = std::make_unique<LocalVarSymbol>(
            scope,
            tmpRefVarName,
            tmpRefType
        );

        auto* const tmpRefVarSymbol = dynamic_cast<LocalVarSymbol*>(
            DiagnosticBag::CreateNoError().Collect(
                scope->DeclareSymbol(std::move(tmpRefVarSymbolOwned))
            )
        );
        ACE_ASSERT(tmpRefVarSymbol);

        const auto tmpRefVarStmt = std::make_shared<const VarStmtSema>(
            tmpRefExpr->GetSrcLocation(),
            tmpRefVarSymbol,
            tmpRefExpr
        );
        stmts.push_back(tmpRefVarStmt);

        const auto tmpRefVarRefExpr = std::make_shared<const StaticVarRefExprSema>(
            tmpRefExpr->GetSrcLocation(),
            scope,
            tmpRefVarSymbol
        );

        const auto tmpRefVarFieldRefExpr = std::make_shared<const FieldVarRefExprSema>(
            lhsExpr->GetSrcLocation(),
            tmpRefVarRefExpr,
            lhsExpr->GetFieldSymbol()
        );

        const auto loweredAssignmentLHSExpr = [&]() -> std::shared_ptr<const IExprSema>
        {
            if (!tmpRefVarFieldRefExpr->GetTypeInfo().Symbol->IsRef())
            {
                return tmpRefVarFieldRefExpr;
            }

            return std::make_shared<const DerefExprSema>(
                tmpRefVarFieldRefExpr->GetSrcLocation(),
                tmpRefVarFieldRefExpr
            );
        }();

        const auto compoundLHSExpr = [&]() -> std::shared_ptr<const IExprSema>
        {
            if (const auto* const refExpr = dynamic_cast<const RefExprSema*>(opLHSExprTemplate.get()))
            {
                return std::make_shared<const RefExprSema>(
                    refExpr->GetSrcLocation(),
                    tmpRefVarFieldRefExpr
                );
            }

            if (const auto* const derefExpr = dynamic_cast<const DerefExprSema*>(opLHSExprTemplate.get()))
            {
                return std::make_shared<const DerefExprSema>(
                    derefExpr->GetSrcLocation(),
                    tmpRefVarFieldRefExpr
                );
            }

            return tmpRefVarFieldRefExpr;
        }();

        const auto userBinaryExpr = std::make_shared<const UserBinaryExprSema>(
            rhsExpr->GetSrcLocation(),
            compoundLHSExpr,
            rhsExpr,
            opSymbol
        );

        const auto assignmentStmt = std::make_shared<const SimpleAssignmentStmtSema>(
            srcLocation,
            loweredAssignmentLHSExpr,
            userBinaryExpr
        );
        stmts.push_back(assignmentStmt);

        return stmts;
    }

    static auto IsStaticVarRefExpr(
        const IExprSema* expr
    ) -> bool
    {
        return dynamic_cast<const StaticVarRefExprSema*>(expr) != nullptr;
    }

    static auto StripCompoundAssignmentWrappers(
        std::shared_ptr<const IExprSema> expr
    ) -> std::shared_ptr<const IExprSema>
    {
        while (true)
        {
            if (const auto* const refExpr = dynamic_cast<const RefExprSema*>(expr.get()))
            {
                expr = refExpr->GetExpr();
                continue;
            }

            if (const auto* const derefExpr = dynamic_cast<const DerefExprSema*>(expr.get()))
            {
                expr = derefExpr->GetExpr();
                continue;
            }

            return expr;
        }
    }

    auto CompoundAssignmentStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtSema>
    {
        if (m_OpSymbol->IsError())
        {
            return std::make_shared<const GroupStmtSema>(
                GetSrcLocation(),
                GetScope(),
                std::vector<std::shared_ptr<const IStmtSema>>{}
            )->CreateLowered({});
        }

        const auto stmts = [&]() -> std::vector<std::shared_ptr<const IStmtSema>>
        {
            const auto unwrappedLHSExpr = StripCompoundAssignmentWrappers(m_LHSExpr);

            if (IsStaticVarRefExpr(unwrappedLHSExpr.get()))
            {
                return CreateStaticVarRefExprLoweredStmts(
                    GetSrcLocation(),
                    unwrappedLHSExpr,
                    m_LHSExpr,
                    m_RHSExpr,
                    m_OpSymbol
                );
            }
            
            const auto* const fieldRefExpr =
                dynamic_cast<const FieldVarRefExprSema*>(unwrappedLHSExpr.get());
            if (fieldRefExpr)
            {
                return CreateFieldVarRefExprLoweredStmts(
                    GetSrcLocation(),
                    GetScope(),
                    fieldRefExpr,
                    m_LHSExpr,
                    m_RHSExpr,
                    m_OpSymbol
                );
            }

            ACE_UNREACHABLE();
        }();

        return std::make_shared<const GroupStmtSema>(
            GetSrcLocation(),
            GetScope(),
            stmts
        )->CreateLowered({});
    }

    auto CompoundAssignmentStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto CompoundAssignmentStmtSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto CompoundAssignmentStmtSema::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto CompoundAssignmentStmtSema::CreateControlFlowInstructions() const -> std::vector<ControlFlowInstruction>
    {
        ACE_UNREACHABLE();
    }
}
