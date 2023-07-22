#include "BoundNodes/Exprs/DerefExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "SpecialIdent.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    DerefExprBoundNode::DerefExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto DerefExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const DerefExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const DerefExprBoundNode>(
            GetSrcLocation(),
            mchCheckedExpr.Value
        ));
    }
    auto DerefExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto DerefExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const DerefExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const DerefExprBoundNode>(
            GetSrcLocation(),
            mchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto DerefExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto DerefExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );
            
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        ACE_ASSERT(typeSymbol->IsRef());

        auto* const type = llvm::PointerType::get(
            emitter.GetIRType(typeSymbol), 
            0
        );

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            type,
            exprEmitResult.Value
        );
        tmps.emplace_back(loadInst, typeSymbol->GetWithoutRef());

        return { loadInst, tmps };
    }

    auto DerefExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        const auto typeInfo = m_Expr->GetTypeInfo();

        auto* const typeSymbol = typeInfo.Symbol;
        ACE_ASSERT(typeSymbol->IsRef());

        return { typeSymbol->GetWithoutRef(), typeInfo.ValueKind };
    }

    auto DerefExprBoundNode::GetExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return m_Expr;
    }
}
