#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace::BoundNode::Expression::VariableReference
{
    class Instance :
        public std::enable_shared_from_this<BoundNode::Expression::VariableReference::Instance>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::VariableReference::Instance>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::VariableReference::Instance>
    {
    public:
        Instance(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression,
            Symbol::Variable::Normal::Instance* const t_variableSymbol
        ) : m_Expression{ t_expression },
            m_VariableSymbol{ t_variableSymbol }
        {
        }
        virtual ~Instance() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpression() const -> std::shared_ptr<const BoundNode::Expression::IBase> { return m_Expression; }
        auto GetVariableSymbol() const -> Symbol::Variable::Normal::Instance* { return m_VariableSymbol; }

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_Expression{};
        Symbol::Variable::Normal::Instance* m_VariableSymbol{};
    };
}
