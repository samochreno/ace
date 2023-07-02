#include "BoundNodes/Exprs/AddressOfExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    AddressOfExprBoundNode::AddressOfExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto AddressOfExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOfExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOfExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const AddressOfExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }
         
        const auto returnValue = std::make_shared<const AddressOfExprBoundNode>(
            m_Expr
        );
        return CreateChanged(returnValue);
    }

    auto AddressOfExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto AddressOfExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const AddressOfExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const AddressOfExprBoundNode>(
            mchLoweredExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto AddressOfExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto AddressOfExprBoundNode::Emit(
        Emitter& t_emitter
    ) const -> ExprEmitResult
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

    auto AddressOfExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return 
        { 
            GetCompilation()->Natives->Pointer.GetSymbol(), 
            ValueKind::R
        };
    }
}
