#include "BoundNode/Expr/DerefAs.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace::BoundNode::Expr
{
    DerefAs::DerefAs(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        ITypeSymbol* const t_typeSymbol
    ) : m_TypeSymbol{ t_typeSymbol },
        m_Expr{ t_expr }
    {
    }

    auto DerefAs::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAs::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAs::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::DerefAs>>>
    {
        auto* const typeSymbol =
            m_Expr->GetTypeInfo().Symbol->GetUnaliased();

        ACE_TRY_ASSERT(
            (typeSymbol == GetCompilation()->Natives->Pointer.GetSymbol()) ||
            (typeSymbol->IsReference())
        );

        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::DerefAs>(
            m_Expr,
            m_TypeSymbol
        );
        return CreateChanged(returnValue);
    }

    auto DerefAs::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto DerefAs::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::DerefAs>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::DerefAs>(
            m_Expr,
            m_TypeSymbol
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto DerefAs::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto DerefAs::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );

        auto* const pointerType = GetCompilation()->Natives->Pointer.GetIRType();

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            pointerType,
            exprEmitResult.Value
        );

        return { loadInst, temporaries };
    }

    auto DerefAs::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
