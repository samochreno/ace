#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Cacheable.hpp"
#include "Assert.hpp"

namespace Ace
{
    UserUnaryExprBoundNode::UserUnaryExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        FunctionSymbol* const opSymbol
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto UserUnaryExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto UserUnaryExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserUnaryExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnaryExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const UserUnaryExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const UserUnaryExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr,
            m_OpSymbol
        );
    }

    auto UserUnaryExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto UserUnaryExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const UserUnaryExprBoundNode>>>
    {
        ACE_TRY(cchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!cchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const UserUnaryExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchCheckedExpr.Value,
            m_OpSymbol
        ));
    }

    auto UserUnaryExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto UserUnaryExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto cchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            m_OpSymbol,
            std::vector{ cchLoweredExpr.Value }
        )->GetOrCreateLowered({}).Value);
    }

    auto UserUnaryExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto UserUnaryExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserUnaryExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
