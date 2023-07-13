#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ReturnStmtBoundNode::ReturnStmtBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& t_optExpr
    ) : m_Scope{ t_scope },
        m_OptExpr{ t_optExpr }
    {
    }

    auto ReturnStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ReturnStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        if (m_OptExpr.has_value())
        {
            AddChildren(children, m_OptExpr.value());
        }

        return children;
    }

    auto ReturnStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ReturnStmtBoundNode>>>
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
        [&](const std::shared_ptr<const IExprBoundNode>& t_expr)
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

        return CreateChanged(std::make_shared<const ReturnStmtBoundNode>(
            m_Scope,
            mchCheckedOptExpr.Value
        ));
    }

    auto ReturnStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto ReturnStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        const auto mchLoweredOptExpr = TransformMaybeChangedOptional(m_OptExpr,
        [&](const std::shared_ptr<const IExprBoundNode>& t_expr)
        {
            return t_expr->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredOptExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ReturnStmtBoundNode>(
            m_Scope,
            mchLoweredOptExpr.Value
        )->GetOrCreateLowered(t_context).Value);
    }

    auto ReturnStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto ReturnStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        if (m_OptExpr.has_value())
        {
            const auto exprEmitResult = m_OptExpr.value()->Emit(t_emitter);
            
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
