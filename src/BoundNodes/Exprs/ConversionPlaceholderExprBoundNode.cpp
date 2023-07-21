#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    ConversionPlaceholderExprBoundNode::ConversionPlaceholderExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& typeInfo
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_TypeInfo{ typeInfo }
    {
    }

    auto ConversionPlaceholderExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto ConversionPlaceholderExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ConversionPlaceholderExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
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
