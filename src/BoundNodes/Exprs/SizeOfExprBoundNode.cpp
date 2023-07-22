#include "BoundNodes/Exprs/SizeOfExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    SizeOfExprBoundNode::SizeOfExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeSymbol{ typeSymbol }
    {
    }

    auto SizeOfExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const SizeOfExprBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SizeOfExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto SizeOfExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const SizeOfExprBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SizeOfExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto SizeOfExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const intTypeSymbol = GetCompilation()->Natives->Int.GetSymbol();
        auto* const intType = emitter.GetIRType(intTypeSymbol);
        auto* const type = emitter.GetIRType(m_TypeSymbol);

        auto* const value = llvm::ConstantInt::get(
            intType,
            emitter.GetModule().getDataLayout().getTypeAllocSize(type)
        );

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(intType);
        tmps.emplace_back(allocaInst, intTypeSymbol);
        
        emitter.GetBlockBuilder().Builder.CreateStore(
            value,
            allocaInst
        );

        return { allocaInst, tmps };
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
