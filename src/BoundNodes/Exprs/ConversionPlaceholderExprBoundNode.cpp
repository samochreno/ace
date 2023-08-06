#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
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

    auto ConversionPlaceholderExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto ConversionPlaceholderExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ConversionPlaceholderExprBoundNode>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholderExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
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
