#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Statement/Expandable.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Statement
{
    class Group : 
        public std::enable_shared_from_this<BoundNode::Statement::Group>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::Statement::IExpandable,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Group, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Group>
    {
    public:
        Group(
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>& t_statements
        ) : m_Scope{ t_scope },
            m_Statements{ t_statements }
        {
        }
        virtual ~Group() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final { ACE_UNREACHABLE(); }
        
        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> m_Statements{};
    };
}
