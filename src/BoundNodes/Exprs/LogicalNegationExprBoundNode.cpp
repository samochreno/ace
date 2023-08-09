#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    LogicalNegationExprBoundNode::LogicalNegationExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LogicalNegationExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegationExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedExpr = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const LogicalNegationExprBoundNode>(
                GetSrcLocation(),
                checkedExpr
            ),
    std::move(        diagnostics),
        };
    }

    auto LogicalNegationExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto LogicalNegationExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const LogicalNegationExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const LogicalNegationExprBoundNode>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto LogicalNegationExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto LogicalNegationExprBoundNode::Emit(
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

        auto* const boolType = GetCompilation()->GetNatives().Bool.GetIRType(
            emitter.GetContext()
        );

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            exprEmitResult.Value
        );

        auto* const negatedValue = emitter.GetBlockBuilder().Builder.CreateXor(
            loadInst,
            1
        );

        auto* const allocaInst = emitter.GetBlockBuilder().Builder.CreateAlloca(
            boolType
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            negatedValue,
            allocaInst
        );

        return { allocaInst, exprEmitResult.Tmps };
    }

    auto LogicalNegationExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R
        };
    }
}
