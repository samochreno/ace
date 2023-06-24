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
    class Return :
        public std::enable_shared_from_this<BoundNode::Statement::Return>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Return, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Return>
    {
    public:
        Return(
            const std::shared_ptr<Scope>& t_scope,
            const std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>& t_optExpression
        );
        virtual ~Return() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Statement::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Return>>> final;
        auto GetOrCreateTypeCheckedStatement(
            const BoundNode::Statement::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Return>> final;
        auto GetOrCreateLoweredStatement(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const BoundNode::Expression::IBase>> m_OptExpression{};
    };
}
