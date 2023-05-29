#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Type/Base.hpp"
#include "TypeInfo.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    class DerefAs :
        public std::enable_shared_from_this<BoundNode::Expression::DerefAs>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::DerefAs>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::DerefAs>
    {
    public:
        DerefAs(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression,
            Symbol::Type::IBase* const t_typeSymbol
        ) : m_TypeSymbol{ t_typeSymbol },
            m_Expression{ t_expression }
        {
        }
        virtual ~DerefAs() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::DerefAs>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::DerefAs>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_Expression{};
        Symbol::Type::IBase* m_TypeSymbol{};
    };
}
