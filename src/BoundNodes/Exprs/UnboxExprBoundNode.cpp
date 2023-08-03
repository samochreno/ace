#include "BoundNodes/Exprs/UnboxExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "MaybeChanged.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    UnboxExprBoundNode::UnboxExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto UnboxExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto UnboxExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UnboxExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);
        
        return children;
    }

    auto UnboxExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const UnboxExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const UnboxExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr
        );
    }

    auto UnboxExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto UnboxExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const UnboxExprBoundNode>>>
    {
        ACE_TRY_ASSERT(m_Expr->GetTypeInfo().Symbol->IsStrongPtr());

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->StrongPtr__value.GetSymbol(),
            std::nullopt,
            { m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        const TypeInfo typeInfo
        {
            functionSymbol->CollectParams().front()->GetType(),
            ValueKind::R,
        };

        ACE_TRY(mchCheckedAndConvertedExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (!mchCheckedAndConvertedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const UnboxExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            mchCheckedAndConvertedExpr.Value
        ));
    }

    auto UnboxExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto UnboxExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->StrongPtr__value.GetSymbol(),
            std::nullopt,
            { mchLoweredExpr.Value->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpr.Value }
        )->GetOrCreateLowered({}).Value);
    }

    auto UnboxExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto UnboxExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UnboxExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr(),
            ValueKind::R,
        };
    }
}
