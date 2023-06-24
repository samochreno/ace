#include "BoundNode/Expr/ConversionPlaceholder.hpp"

namespace Ace::BoundNode::Expr
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
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::ConversionPlaceholder>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::ConversionPlaceholder>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::Emit(
        Emitter& t_emitter
    ) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto ConversionPlaceholder::GetTypeInfo() const -> TypeInfo
    {
        return m_TypeInfo;
    }
}
