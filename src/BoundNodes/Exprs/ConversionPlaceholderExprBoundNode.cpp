#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    ConversionPlaceholderExprBoundNode::ConversionPlaceholderExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& typeInfo
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeInfo{ typeInfo }
    {
    }

    auto ConversionPlaceholderExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
