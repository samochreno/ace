#pragma once

#include <memory>

#include "BoundNode/Statement/Base.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement
{
    class BlockEnd : 
        public std::enable_shared_from_this<BoundNode::Statement::BlockEnd>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::BlockEnd, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::BlockEnd>
    {
    public:
        BlockEnd(const std::shared_ptr<Scope>& t_selfScope) 
            : m_SelfScope{ t_selfScope }
        {
        }
        virtual ~BlockEnd() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_SelfScope->GetParent().value(); }
        auto GetSelfScope() const -> std::shared_ptr<Scope> { return m_SelfScope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::BlockEnd>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::BlockEnd>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
    };
}
