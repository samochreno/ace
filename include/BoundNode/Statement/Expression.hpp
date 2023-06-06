#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement
{
    class Expression : 
        public std::enable_shared_from_this<BoundNode::Statement::Expression>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Expression, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Expression>
    {
    public:
        Expression(const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression) 
            : m_Expression{ t_expression }
        {
        }
        virtual ~Expression() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Expression>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Expression>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_Expression{};
    };
}
