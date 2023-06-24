#include "BoundNode/Stmt/Var.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "BoundNode/Stmt/Assignment/Normal.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Stmt
{
    Var::Var(
        Symbol::Var::Local* const t_symbol,
        const std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>& t_optAssignedExpr
    ) : m_Symbol{ t_symbol },
        m_OptAssignedExpr{ t_optAssignedExpr }
    {
    }

    auto Var::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Var::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        if (m_OptAssignedExpr.has_value())
        {
            AddChildren(children, m_OptAssignedExpr.value());
        }

        return children;
    }

    auto Var::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Var>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(mchConvertedAndCheckedOptAssignedExpr, CreateImplicitlyConvertedAndTypeCheckedOptional(
            m_OptAssignedExpr,
            TypeInfo{ m_Symbol->GetType(), ValueKind::R }
        ));

        if (!mchConvertedAndCheckedOptAssignedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Var>(
            m_Symbol,
            mchConvertedAndCheckedOptAssignedExpr.Value
        );
        return CreateChanged(returnValue);
    }

    auto Var::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Var::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Var>>
    {
        const auto mchLoweredOptAssignedExpr = TransformMaybeChangedOptional(m_OptAssignedExpr,
        [](const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr)
        {
            return t_expr->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredOptAssignedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Var>(
            m_Symbol,
            mchLoweredOptAssignedExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Var::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Var::Emit(Emitter& t_emitter) const -> void
    {
        if (!m_OptAssignedExpr.has_value())
        {
            return;
        }

        const auto variableReferenceExpr = std::make_shared<const BoundNode::Expr::VarReference::Static>(
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions, references can be initialized too.
        const auto assignmentStmt = std::make_shared<const BoundNode::Stmt::Assignment::Normal>(
            variableReferenceExpr,
            m_OptAssignedExpr.value()
        );

        assignmentStmt->Emit(t_emitter);
    }

    auto Var::GetSymbol() const -> Symbol::Var::Local*
    {
        return m_Symbol;
    }
}
