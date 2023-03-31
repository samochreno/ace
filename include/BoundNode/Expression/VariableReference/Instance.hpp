#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/VariableReference/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Symbol/Variable/Base.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace::BoundNode::Expression::VariableReference
{
    class Instance :
        public std::enable_shared_from_this<BoundNode::Expression::VariableReference::Instance>,
        public virtual BoundNode::Expression::VariableReference::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::VariableReference::Instance>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::VariableReference::Instance>
    {
    public:
        Instance(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression,
            Symbol::Variable::IBase* const t_variableSymbol
        ) : m_Expression{ t_expression },
            m_VariableSymbol{ t_variableSymbol }
        {
        }
        virtual ~Instance() = default;

        auto GetScope() const -> Scope* final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetVariableSymbol() const -> Symbol::Variable::IBase* final { return m_VariableSymbol; }

        auto GetExpression() const -> std::shared_ptr<const BoundNode::Expression::IBase> { return m_Expression; }

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_Expression{};
        Symbol::Variable::IBase* m_VariableSymbol{};
    };
}
