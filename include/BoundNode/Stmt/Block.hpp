#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Stmt/Expandable.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt
{
    class Block : 
        public std::enable_shared_from_this<BoundNode::Stmt::Block>,
        public virtual BoundNode::Stmt::IBase,
        public virtual BoundNode::Stmt::IExpandable,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Block, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Block>
    {
    public:
        Block(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>& t_stmts
        );
        virtual ~Block() = default;
        
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Block>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Block>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_SelfScope;
        std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> m_Stmts{};
    };
}
