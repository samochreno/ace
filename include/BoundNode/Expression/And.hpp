#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    class And :
        public std::enable_shared_from_this<BoundNode::Expression::And>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::And>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::And>
    {
    public:
        And(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_rhsExpression
        ) : m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression }
        {
        }
        virtual ~And() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_LHSExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::And>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::And>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;
    
    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_LHSExpression;
        std::shared_ptr<const BoundNode::Expression::IBase> m_RHSExpression;
    };
}
