#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement::Assignment
{
    class Normal :
        public std::enable_shared_from_this<BoundNode::Statement::Assignment::Normal>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Assignment::Normal, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Assignment::Normal>
    {
    public:
        Normal(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_rhsExpression
        ) : m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> Scope* final { return m_LHSExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assignment::Normal>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assignment::Normal>>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const BoundNode::Expression::IBase> m_RHSExpression{};
    };
}
