#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Symbol/Type/Struct.hpp"
#include "TypeInfo.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    class StructConstruction :
        public std::enable_shared_from_this<BoundNode::Expression::StructConstruction>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::StructConstruction>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::StructConstruction>
    {
    public:
        struct Argument
        {
            Symbol::Variable::Normal::Instance* Symbol{};
            std::shared_ptr<const BoundNode::Expression::IBase> Value{};
        };

        StructConstruction(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Type::Struct* const t_structSymbol,
            const std::vector<Argument>& t_arguments
        ) : m_Scope{ t_scope },
            m_StructSymbol{ t_structSymbol },
            m_Arguments{ t_arguments }
        {
        }
        virtual ~StructConstruction() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::StructConstruction>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::StructConstruction>>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Type::Struct* m_StructSymbol{};
        std::vector<Argument> m_Arguments{};
    };
}
