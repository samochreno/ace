#include "BoundNode/Stmt/Return.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Stmt
{
    Return::Return(
        const std::shared_ptr<Scope>& t_scope,
        const std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>& t_optExpr
    ) : m_Scope{ t_scope },
        m_OptExpr{ t_optExpr }
    {
    }

    auto Return::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Return::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        if (m_OptExpr.has_value())
        {
            AddChildren(children, m_OptExpr.value());
        }

        return children;
    }

    auto Return::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Return>>>
    {
        const bool isFunctionTypeVoid = 
            t_context.ParentFunctionTypeSymbol == 
            GetCompilation()->Natives->Void.GetSymbol();

        ACE_TRY_ASSERT(m_OptExpr.has_value() != isFunctionTypeVoid);

        if (m_OptExpr.has_value())
        {
            auto* const exprTypeSymbol = 
                m_OptExpr.value()->GetTypeInfo().Symbol->GetUnaliased();

            const bool isExprTypeVoid = 
                exprTypeSymbol ==
                GetCompilation()->Natives->Void.GetSymbol();

            ACE_TRY_ASSERT(!isExprTypeVoid);
        }

        ACE_TRY(mchCheckedOptExpr, TransformExpectedMaybeChangedOptional(m_OptExpr,
        [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr)
        {
            return CreateImplicitlyConvertedAndTypeChecked(
                t_expr,
                { t_context.ParentFunctionTypeSymbol, ValueKind::R }
            );
        }));

        if (!mchCheckedOptExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Return>(
            m_Scope,
            mchCheckedOptExpr.Value
        );
        
        return CreateChanged(returnValue);
    }

    auto Return::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Return::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Return>>
    {
        const auto mchLoweredOptExpr = TransformMaybeChangedOptional(m_OptExpr,
        [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr)
        {
            return t_expr->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredOptExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Return>(
            m_Scope,
            mchLoweredOptExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Return::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Return::Emit(Emitter& t_emitter) const -> void
    {
        if (m_OptExpr.has_value())
        {
            const auto exprEmitResult =
                m_OptExpr.value()->Emit(t_emitter);
            
            auto* const typeSymbol = m_OptExpr.value()->GetTypeInfo().Symbol;
            auto* const type = t_emitter.GetIRType(typeSymbol);

            auto* const allocaInst =
                t_emitter.GetBlockBuilder().Builder.CreateAlloca(type);

            t_emitter.EmitCopy(
                allocaInst,
                exprEmitResult.Value,
                typeSymbol
            );

            t_emitter.EmitDropTemporaries(exprEmitResult.Temporaries);
            t_emitter.EmitDropLocalVarsBeforeStmt(this);
            
            auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
                type,
                allocaInst
            );

            t_emitter.GetBlockBuilder().Builder.CreateRet(loadInst);
        }
        else
        {
            t_emitter.EmitDropLocalVarsBeforeStmt(this);

            t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
        }
    }
}
