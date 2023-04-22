#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Statement/Expandable.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement
{
    class Block : 
        public std::enable_shared_from_this<BoundNode::Statement::Block>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::Statement::IExpandable,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Block, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Block>
    {
    public:
        Block(
            Scope* const t_selfScope,
            const std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>& t_statements
        ) : m_SelfScope{ t_selfScope },
            m_Statements{ t_statements }
        {
        }
        virtual ~Block() = default;
        
        auto GetScope() const -> Scope* final { return m_SelfScope->GetParent().value(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Block>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Block>>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> final;

        auto IsEndReachableWithoutReturn() const -> bool;

    private:
        Scope* m_SelfScope{};
        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> m_Statements{};
    };
}
