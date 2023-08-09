#include "BoundNodes/Exprs/SizeOfExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

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

    auto SizeOfExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto SizeOfExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const SizeOfExprBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto SizeOfExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto SizeOfExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const SizeOfExprBoundNode>
    {
        return shared_from_this();
    }

    auto SizeOfExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto SizeOfExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const intTypeSymbol = GetCompilation()->GetNatives().Int.GetSymbol();
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
            GetCompilation()->GetNatives().Int.GetSymbol(),
            ValueKind::R
        };
    }
}
