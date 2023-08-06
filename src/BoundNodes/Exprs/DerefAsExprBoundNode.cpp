#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    DerefAsExprBoundNode::DerefAsExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        ITypeSymbol* const typeSymbol
    ) : m_SrcLocation{ srcLocation },
        m_TypeSymbol{ typeSymbol },
        m_Expr{ expr }
    {
    }

    auto DerefAsExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefAsExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAsExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const DerefAsExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol->GetUnaliased();

        const bool isRef = typeSymbol->IsRef();
        const bool isPtr =
            typeSymbol == GetCompilation()->GetNatives()->Ptr.GetSymbol();

        if (!isRef && !isPtr)
        {
            diagnostics.Add(CreateExpectedDerefableExprError(GetSrcLocation()));
        }

        const auto dgnCheckedExpr = m_Expr->CreateTypeCheckedExpr({});
        diagnostics.Add(dgnCheckedExpr);

        if (dgnCheckedExpr.Unwrap() == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const DerefAsExprBoundNode>(
                GetSrcLocation(),
                m_Expr,
                m_TypeSymbol
            ),
            diagnostics,
        };
    }

    auto DerefAsExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto DerefAsExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const DerefAsExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const DerefAsExprBoundNode>(
            GetSrcLocation(),
            m_Expr,
            m_TypeSymbol
        )->CreateLowered({});
    }

    auto DerefAsExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto DerefAsExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const ptrType = GetCompilation()->GetNatives()->Ptr.GetIRType();

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            ptrType,
            exprEmitResult.Value
        );

        return { loadInst, tmps };
    }

    auto DerefAsExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
