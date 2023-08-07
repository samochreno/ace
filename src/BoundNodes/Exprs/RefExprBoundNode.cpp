#include "BoundNodes/Exprs/RefExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    RefExprBoundNode::RefExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto RefExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto RefExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto RefExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto RefExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const RefExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const RefExprBoundNode>(
                GetSrcLocation(),
                checkedExpr
            ),
            diagnostics,
        };
    }

    auto RefExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto RefExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const RefExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const RefExprBoundNode>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto RefExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto RefExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        ); 

        auto* const allocaInst = emitter.GetBlockBuilder().Builder.CreateAlloca(
            exprEmitResult.Value->getType()
        );
        tmps.emplace_back(
            allocaInst, m_Expr->GetTypeInfo().Symbol->GetWithRef()
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            exprEmitResult.Value,
            allocaInst
        );

        return { allocaInst, tmps };
    }

    auto RefExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        const auto scope = typeSymbol->GetScope();

        return { typeSymbol->GetWithRef(), ValueKind::R };
    }
}
