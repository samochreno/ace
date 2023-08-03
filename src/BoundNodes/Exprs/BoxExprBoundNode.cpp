#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Cacheable.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    BoxExprBoundNode::BoxExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto BoxExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto BoxExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BoxExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto BoxExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto BoxExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const BoxExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const BoxExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr
        );
    }

    auto BoxExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto BoxExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const BoxExprBoundNode>>> 
    {
        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->StrongPtr__new.GetSymbol(),
            std::nullopt,
            { m_Expr->GetTypeInfo().Symbol->GetWithoutRef() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        const TypeInfo typeInfo
        {
            functionSymbol->CollectParams().front()->GetType(),
            ValueKind::R,
        };

        ACE_TRY(cchCheckedAndConvertedExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (!cchCheckedAndConvertedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const BoxExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchCheckedAndConvertedExpr.Value
        ));
    }
    
    auto BoxExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto BoxExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StaticFunctionCallExprBoundNode>> 
    {
        const auto cchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives()->StrongPtr__new.GetSymbol(),
            std::nullopt,
            { cchLoweredExpr.Value->GetTypeInfo().Symbol->GetWithoutRef() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            functionSymbol,
            std::vector{ cchLoweredExpr.Value }
        )->GetOrCreateLowered({}).Value);
    }

    auto BoxExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto BoxExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto BoxExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        { 
            m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithStrongPtr(),
            ValueKind::R
        };
    }
}
