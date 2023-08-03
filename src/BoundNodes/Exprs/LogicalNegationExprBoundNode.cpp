#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

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
    LogicalNegationExprBoundNode::LogicalNegationExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
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

    auto LogicalNegationExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const LogicalNegationExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const LogicalNegationExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr
        );
    }

    auto LogicalNegationExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const LogicalNegationExprBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives()->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(cchConvertedAndCheckedExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (!cchConvertedAndCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedExpr.Value
        ));
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        const auto cchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!cchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
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

        auto* const boolType = GetCompilation()->GetNatives()->Bool.GetIRType();

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
        return { GetCompilation()->GetNatives()->Bool.GetSymbol(), ValueKind::R };
    }
}
