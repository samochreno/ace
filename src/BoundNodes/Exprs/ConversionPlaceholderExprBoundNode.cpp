#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    ConversionPlaceholderExprBoundNode::ConversionPlaceholderExprBoundNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const TypeInfo& t_typeInfo
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_TypeInfo{ t_typeInfo }
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
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::Emit(
        Emitter& t_emitter
    ) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return m_TypeInfo;
    }
}
