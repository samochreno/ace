#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Symbol/Variable/Base.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace::BoundNode::Expression::VariableReference
{
    class Static :
        public std::enable_shared_from_this<BoundNode::Expression::VariableReference::Static>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::VariableReference::Static>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::VariableReference::Static>
    {
    public:
        Static(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Variable::IBase* const t_variableSymbol
        ) : m_Scope{ t_scope },
            m_VariableSymbol{ t_variableSymbol }
        {
        }
        virtual ~Static() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Static>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Static>>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetVariableSymbol() const -> Symbol::Variable::IBase* { return m_VariableSymbol; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Variable::IBase* m_VariableSymbol{};
    };
}
