#include "BoundNode/Expression/ConversionPlaceholder.hpp"

namespace Ace::BoundNode::Expression
{
    ConversionPlaceholder::ConversionPlaceholder(
        const std::shared_ptr<Scope>& t_scope,
        const TypeInfo& t_typeInfo
    ) : m_Scope{ t_scope },
        m_TypeInfo{ t_typeInfo }
    {
    }

    auto ConversionPlaceholder::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ConversionPlaceholder::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::ConversionPlaceholder>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::ConversionPlaceholder>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::Emit(
        Emitter& t_emitter
    ) const -> ExpressionEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetTypeInfo() const -> TypeInfo
    {
        return m_TypeInfo;
    }
}
