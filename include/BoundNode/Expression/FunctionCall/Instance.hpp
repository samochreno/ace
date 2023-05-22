#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Function.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression::FunctionCall
{
    class Instance :
        public std::enable_shared_from_this<BoundNode::Expression::FunctionCall::Instance>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::FunctionCall::Instance>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::FunctionCall::Instance>
    {
    public:
        Instance(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression,
            Symbol::Function* const t_functionSymbol,
            const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_arguments
        ) : m_Expression{ t_expression },
            m_FunctionSymbol{ t_functionSymbol },
            m_Arguments{ t_arguments }
        {
        }
        virtual ~Instance() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Instance>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Instance>>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_Expression{};
        Symbol::Function* m_FunctionSymbol{};
        std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> m_Arguments{};
    };
}
