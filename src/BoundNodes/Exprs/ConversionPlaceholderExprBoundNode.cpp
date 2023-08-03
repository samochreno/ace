#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    ConversionPlaceholderExprBoundNode::ConversionPlaceholderExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& typeInfo
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeInfo{ typeInfo }
    {
    }

    auto ConversionPlaceholderExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto ConversionPlaceholderExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ConversionPlaceholderExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ConversionPlaceholderExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ConversionPlaceholderExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const ConversionPlaceholderExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            GetTypeInfo()
        );
    }

    auto ConversionPlaceholderExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return m_TypeInfo;
    }
}
