#include "BoundNodes/Exprs/OrExprBoundNode.hpp"

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
    OrExprBoundNode::OrExprBoundNode(
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

    auto OrExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto OrExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto OrExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto OrExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto OrExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const OrExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const OrExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_LHSExpr,
            m_RHSExpr
        );
    }

    auto OrExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto OrExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const OrExprBoundNode>>>
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

        return CreateChanged(std::make_shared<const OrExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedLHSExpr.Value,
            cchConvertedAndCheckedRHSExpr.Value
        ));
    }

    auto OrExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto OrExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const OrExprBoundNode>>
    {
        const auto cchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto cchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        if (
            !cchLoweredLHSExpr.IsChanged &&
            !cchLoweredRHSExpr.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        return CreateChanged(std::make_shared<const OrExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredLHSExpr.Value,
            cchLoweredRHSExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto OrExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto OrExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const boolType = GetCompilation()->GetNatives()->Bool.GetIRType();

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(boolType);

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

        emitter.GetBlockBuilder().Builder.CreateStore(
            lhsLoadInst,
            allocaInst
        );

        auto falseBlockBuilder = std::make_unique<BlockBuilder>(
            GetCompilation()->GetLLVMContext(),
            emitter.GetFunction()
        );

        auto endBlockBuilder = std::make_unique<BlockBuilder>(
            GetCompilation()->GetLLVMContext(),
            emitter.GetFunction()
        );

        emitter.GetBlockBuilder().Builder.CreateCondBr(
            lhsLoadInst,
            endBlockBuilder->Block,
            falseBlockBuilder->Block
        );

        emitter.SetBlockBuilder(std::move(falseBlockBuilder));

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

    auto OrExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R 
        };
    }
}
