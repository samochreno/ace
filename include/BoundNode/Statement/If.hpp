#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Statement
{
    class If :
        public std::enable_shared_from_this<BoundNode::Statement::If>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::If, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Group>
    {
    public:
        If(
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_conditions,
            const std::vector<std::shared_ptr<const BoundNode::Statement::Block>>& t_bodies
        ) : m_Scope{ t_scope },
            m_Conditions{ t_conditions },
            m_Bodies{ t_bodies }
        {
        }
        virtual ~If() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::If>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final { ACE_UNREACHABLE(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> m_Conditions{};
        std::vector<std::shared_ptr<const BoundNode::Statement::Block>> m_Bodies{};
    };
}
