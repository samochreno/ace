#include "BoundNode/Stmt/Assignment/Compound.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Expr//Reference.hpp"
#include "BoundNode/Expr//Dereference.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "BoundNode/Expr/VarReference/Instance.hpp"
#include "BoundNode/Expr/UserBinary.hpp"
#include "BoundNode/Stmt/Group.hpp"
#include "BoundNode/Stmt/Block.hpp"
#include "BoundNode/Stmt/Expr.hpp"
#include "BoundNode/Stmt/Assignment/Normal.hpp"
#include "BoundNode/Stmt/Var.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::BoundNode::Stmt::Assignment
{
    Compound::Compound(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_lhsExpr,
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_rhsExpr,
        FunctionSymbol* const t_operatorSymbol
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_OperatorSymbol{ t_operatorSymbol }
    {
    }

    auto Compound::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto Compound::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto Compound::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Assignment::Compound>>>
    {
        const auto argTypeInfos =
            m_OperatorSymbol->CollectArgTypeInfos();
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

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Assignment::Compound>(
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value,
            m_OperatorSymbol
        );

        return CreateChanged(returnValue);
    }

    auto Compound::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Compound::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Group>>
    {
        std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> stmts{};

        const bool isStaticVarReferenceExpr = [&]() -> bool
        {
            auto* nextExpr = m_LHSExpr.get();
            while (const auto* const expr = dynamic_cast<const BoundNode::Expr::Dereference*>(nextExpr))
            {
                nextExpr = expr->GetExpr().get();
            }

            return dynamic_cast<const BoundNode::Expr::VarReference::Static*>(nextExpr) != nullptr;
        }();

        if (isStaticVarReferenceExpr)
        {
            // From:
            // lhs += rhs;
            // 
            // To:
            // lhs = lhs + rhs;

            const auto userBinaryExpr = std::make_shared<const BoundNode::Expr::UserBinary>(
                m_LHSExpr,
                m_RHSExpr,
                m_OperatorSymbol
            );

            const auto assignmentStmt = std::make_shared<const BoundNode::Stmt::Assignment::Normal>(
                m_LHSExpr,
                userBinaryExpr
            );
            stmts.push_back(assignmentStmt);
        }
        else if (auto instanceVarReferenceExpr = dynamic_cast<const BoundNode::Expr::VarReference::Instance*>(m_LHSExpr.get()))
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

            std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> blockStmts{};
            const std::shared_ptr<Scope> blockScope = GetScope()->GetOrCreateChild({});

            const auto tmpRefExpr = [&]() -> std::shared_ptr<const BoundNode::Expr::IBase>
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

                        return std::make_shared<const BoundNode::Expr::Reference>(expr);
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

                        const auto tmpVarStmt = std::make_shared<const BoundNode::Stmt::Var>(
                            tmpVarSymbol,
                            expr
                        );
                        blockStmts.push_back(tmpVarStmt);

                        const auto tmpVarReferenceExpr = std::make_shared<const BoundNode::Expr::VarReference::Static>(
                            blockScope,
                            tmpVarSymbol
                        );

                        return std::make_shared<const BoundNode::Expr::Reference>(tmpVarReferenceExpr);
                    }

                    default:
                    {
                        ACE_UNREACHABLE();
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

            const auto tmpRefVarStmt = std::make_shared<const BoundNode::Stmt::Var>(
                tmpRefVarSymbol,
                tmpRefExpr
            );
            blockStmts.push_back(tmpRefVarStmt);

            const auto tmpRefVarReferenceExpr = std::make_shared<const BoundNode::Expr::VarReference::Static>(
                blockScope,
                tmpRefVarSymbol
            );

            const auto tmpRefVarFieldReferenceExpr = std::make_shared<const BoundNode::Expr::VarReference::Instance>(
                tmpRefVarReferenceExpr,
                instanceVarReferenceExpr->GetVarSymbol()
            );

            const auto userBinaryExpr = std::make_shared<const BoundNode::Expr::UserBinary>(
                tmpRefVarFieldReferenceExpr,
                m_RHSExpr,
                m_OperatorSymbol
            );

            const auto assignmentStmt = std::make_shared<const BoundNode::Stmt::Assignment::Normal>(
                tmpRefVarFieldReferenceExpr,
                userBinaryExpr
            );
            blockStmts.push_back(assignmentStmt);

            const auto blockStmt = std::make_shared<const BoundNode::Stmt::Block>(
                blockScope,
                blockStmts
            );
            stmts.push_back(blockStmt);
        }
        else
        {
            ACE_UNREACHABLE();
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Group>(
            GetScope(),
            stmts
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Compound::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Compound::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
