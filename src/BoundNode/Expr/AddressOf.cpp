#include "BoundNode/Expr//AddressOf.hpp"

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
    AddressOf::AddressOf(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto AddressOf::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOf::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOf::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::AddressOf>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }
         
        const auto returnValue = std::make_shared<const BoundNode::Expr::AddressOf>(
            m_Expr
        );
        return CreateChanged(returnValue);
    }

    auto AddressOf::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto AddressOf::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::AddressOf>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::AddressOf>(
            mchLoweredExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto AddressOf::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto AddressOf::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );

        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        auto* const type = llvm::PointerType::get(
            t_emitter.GetIRType(typeSymbol),
            0
        );

        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(type);
        temporaries.emplace_back(
            allocaInst, 
            GetCompilation()->Natives->Pointer.GetSymbol()
        );

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            exprEmitResult.Value, 
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto AddressOf::GetTypeInfo() const -> TypeInfo
    {
        return 
        { 
            GetCompilation()->Natives->Pointer.GetSymbol(), 
            ValueKind::R
        };
    }
}
