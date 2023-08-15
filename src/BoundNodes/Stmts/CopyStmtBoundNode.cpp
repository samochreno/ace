#include "BoundNodes/Stmts/CopyStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    CopyStmtBoundNode::CopyStmtBoundNode(
        const SrcLocation& srcLocation,
        ISizedTypeSymbol* const typeSymbol,
        const std::shared_ptr<const IExprBoundNode>& srcExpr,
        const std::shared_ptr<const IExprBoundNode>& dstExpr
    ) : m_SrcLocation{ srcLocation },
        m_TypeSymbol{ typeSymbol },
        m_SrcExpr{ srcExpr },
        m_DstExpr{ dstExpr }
    {
    }

    auto CopyStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CopyStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SrcExpr->GetScope();
    }

    auto CopyStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_SrcExpr);
        AddChildren(children, m_DstExpr);

        return children;
    }

    auto CopyStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const CopyStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundSrcExpr =
            diagnostics.Collect(m_SrcExpr->CreateTypeCheckedExpr({}));
        const auto boundDstExpr =
            diagnostics.Collect(m_DstExpr->CreateTypeCheckedExpr({}));

        auto* const ptrTypeSymbol =
            GetCompilation()->GetNatives().Ptr.GetSymbol();

        if (boundSrcExpr->GetTypeInfo().Symbol != ptrTypeSymbol)
        {
            diagnostics.Add(CreateExpectedPtrExprError(
                boundSrcExpr->GetSrcLocation()
            ));
        }

        if (boundDstExpr->GetTypeInfo().Symbol != ptrTypeSymbol)
        {
            diagnostics.Add(CreateExpectedPtrExprError(
                boundDstExpr->GetSrcLocation()
            ));
        }

        if (
            (boundSrcExpr == m_SrcExpr) &&
            (boundDstExpr == m_DstExpr)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const CopyStmtBoundNode>(
                m_SrcLocation,
                m_TypeSymbol,
                boundSrcExpr,
                boundDstExpr
            ),
            std::move(diagnostics),
        };
    }

    auto CopyStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto CopyStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const CopyStmtBoundNode>
    {
        const auto loweredSrcExpr = m_SrcExpr->CreateLoweredExpr(context);
        const auto loweredDstExpr = m_DstExpr->CreateLoweredExpr(context);

        if (
            (loweredSrcExpr == m_SrcExpr) &&
            (loweredDstExpr == m_DstExpr)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const CopyStmtBoundNode>(
            m_SrcLocation,
            m_TypeSymbol,
            loweredSrcExpr,
            loweredDstExpr
        );
    }

    auto CopyStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto CopyStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropData> tmps{};

        const auto srcExprEmitResult = m_SrcExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(srcExprEmitResult.Tmps),
            end  (srcExprEmitResult.Tmps)
        );

        const auto dstExprEmitResult = m_DstExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(dstExprEmitResult.Tmps),
            end  (dstExprEmitResult.Tmps)
        );

        auto* const glueSymbol = m_TypeSymbol->GetCopyGlue().value();

        emitter.GetBlockBuilder().Builder.CreateCall(
            emitter.GetFunctionMap().at(glueSymbol),
            { dstExprEmitResult.Value, srcExprEmitResult.Value }
        );
    }

    auto CopyStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return {};
    }
}
