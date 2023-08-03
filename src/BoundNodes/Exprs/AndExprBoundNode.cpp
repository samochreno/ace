#include "BoundNodes/Exprs/AndExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    AndExprBoundNode::AndExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto AndExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto AndExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AndExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto AndExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto AndExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const AndExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const AndExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_LHSExpr,
            m_RHSExpr
        );
    }

    auto AndExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto AndExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const AndExprBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(cchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            typeInfo
        ));

        ACE_TRY(cchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            typeInfo
        ));

        if (
            !cchConvertedAndCheckedLHSExpr.IsChanged &&
            !cchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AndExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedLHSExpr.Value,
            cchConvertedAndCheckedRHSExpr.Value
        ));
    }

    auto AndExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }


    auto AndExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const AndExprBoundNode>>
    {
        const auto cchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto cchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        if (
            !cchLoweredLHSExpr.IsChanged && 
            !cchLoweredRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AndExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredLHSExpr.Value,
            cchLoweredRHSExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto AndExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto AndExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const boolType = GetCompilation()->GetNatives()->Bool.GetIRType();

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(boolType);

        emitter.GetBlockBuilder().Builder.CreateStore(
            llvm::ConstantInt::get(boolType, 0),
            allocaInst
        );

        const auto lhsEmitResult = m_LHSExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(lhsEmitResult.Tmps),
            end  (lhsEmitResult.Tmps)
        );

        auto* const lhsLoadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            lhsEmitResult.Value
        );

        auto trueBlockBuilder = std::make_unique<BlockBuilder>(
            GetCompilation()->GetLLVMContext(),
            emitter.GetFunction()
        );

        auto endBlockBuilder = std::make_unique<BlockBuilder>(
            GetCompilation()->GetLLVMContext(),
            emitter.GetFunction()
        );

        emitter.GetBlockBuilder().Builder.CreateCondBr(
            lhsLoadInst,
            trueBlockBuilder->Block,
            endBlockBuilder->Block
        );

        emitter.SetBlockBuilder(std::move(trueBlockBuilder));

        const auto rhsEmitResult = m_RHSExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(rhsEmitResult.Tmps),
            end  (rhsEmitResult.Tmps)
        );

        auto* const rhsLoadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            rhsEmitResult.Value
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            rhsLoadInst,
            allocaInst
        );

        emitter.GetBlockBuilder().Builder.CreateBr(endBlockBuilder->Block);

        emitter.SetBlockBuilder(std::move(endBlockBuilder));

        return { allocaInst, tmps };
    }

    auto AndExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R
        };
    }
}
