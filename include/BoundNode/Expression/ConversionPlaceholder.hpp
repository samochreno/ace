#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "TypeInfo.hpp"
#include "Scope.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    class ConversionPlaceholder :
        public std::enable_shared_from_this<BoundNode::Expression::ConversionPlaceholder>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::ConversionPlaceholder>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::ConversionPlaceholder>
    {
    public:
        ConversionPlaceholder(
            const std::shared_ptr<Scope>& t_scope,
            const TypeInfo& t_typeInfo
        ) : m_Scope{ t_scope },
            m_TypeInfo{ t_typeInfo }
        {
        }
        virtual ~ConversionPlaceholder() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final { ACE_UNREACHABLE(); }
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::ConversionPlaceholder>>> final { ACE_UNREACHABLE(); }
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { ACE_UNREACHABLE(); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::ConversionPlaceholder>> final { ACE_UNREACHABLE(); }
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> final { ACE_UNREACHABLE(); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final { ACE_UNREACHABLE(); }

        auto GetTypeInfo() const -> TypeInfo final { return m_TypeInfo; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        TypeInfo m_TypeInfo;
    };
}
