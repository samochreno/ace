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
    class Static :
        public std::enable_shared_from_this<BoundNode::Expression::FunctionCall::Static>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::FunctionCall::Static>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::FunctionCall::Static>
    {
    public:
        Static(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Function* const t_functionSymbol,
            const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_arguments
        ) : m_Scope{ t_scope },
            m_FunctionSymbol{ t_functionSymbol },
            m_Arguments{ t_arguments }
        {
        }
        virtual ~Static() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Function* m_FunctionSymbol{};
        std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> m_Arguments{};
    };
}
