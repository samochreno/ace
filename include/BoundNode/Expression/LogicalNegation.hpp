#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    class LogicalNegation :
        public std::enable_shared_from_this<BoundNode::Expression::LogicalNegation>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::LogicalNegation>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::LogicalNegation>
    {
    public:
        LogicalNegation(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression
        );
        virtual ~LogicalNegation() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::LogicalNegation>>> final;
        auto GetOrCreateTypeCheckedExpression(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::LogicalNegation>> final;
        auto GetOrCreateLoweredExpression(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_Expression{};
    };
}
