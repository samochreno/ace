#include "BoundNodes/Exprs/SizeOfExprBoundNode.hpp"

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
    SizeOfExprBoundNode::SizeOfExprBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* const t_typeSymbol
    ) : m_Scope{ t_scope },
        m_TypeSymbol{ t_typeSymbol }
    {
    }

    auto SizeOfExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOfExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto SizeOfExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const SizeOfExprBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SizeOfExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto SizeOfExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const SizeOfExprBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SizeOfExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto SizeOfExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        auto* const intTypeSymbol = GetCompilation()->Natives->Int.GetSymbol();
        auto* const intType = t_emitter.GetIRType(intTypeSymbol);
        auto* const type = t_emitter.GetIRType(m_TypeSymbol);

        auto* const value = llvm::ConstantInt::get(
            intType,
            t_emitter.GetModule().getDataLayout().getTypeAllocSize(type)
        );

        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(intType);
        temporaries.emplace_back(allocaInst, intTypeSymbol);
        
        t_emitter.GetBlockBuilder().Builder.CreateStore(
            value,
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto SizeOfExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->Natives->Int.GetSymbol(),
            ValueKind::R
        };
    }
}
