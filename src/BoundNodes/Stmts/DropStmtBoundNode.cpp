#include "BoundNodes/Stmts/DropStmtBoundNode.hpp"

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
    DropStmtBoundNode::DropStmtBoundNode(
        const SrcLocation& srcLocation,
        ISizedTypeSymbol* const typeSymbol,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeSymbol{ typeSymbol },
        m_Expr{ expr }
    {
    }

    auto DropStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DropStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DropStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DropStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const DropStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        auto* const ptrTypeSymbol =
            GetCompilation()->GetNatives().Ptr.GetSymbol();

        if (boundExpr->GetTypeInfo().Symbol != ptrTypeSymbol)
        {
            diagnostics.Add(CreateExpectedPtrExprError(
                boundExpr->GetSrcLocation()
            ));
        }

        if (boundExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const DropStmtBoundNode>(
                m_SrcLocation,
                m_TypeSymbol,
                boundExpr
            ),
            std::move(diagnostics),
        };
    }

    auto DropStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto DropStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const DropStmtBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr(context);

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const DropStmtBoundNode>(
            m_SrcLocation,
            m_TypeSymbol,
            loweredExpr
        );
    }

    auto DropStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto DropStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const glueSymbol = m_TypeSymbol->GetDropGlue().value();

        emitter.GetBlockBuilder().Builder.CreateCall(
            emitter.GetFunctionMap().at(glueSymbol),
            { exprEmitResult.Value }
        );
    }

    auto DropStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return {};
    }
}
