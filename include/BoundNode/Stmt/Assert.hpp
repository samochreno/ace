#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Stmt/Group.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Stmt
{
    class Assert : 
        public std::enable_shared_from_this<BoundNode::Stmt::Assert>,
        public virtual BoundNode::Stmt::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Assert, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Group>
    {
    public:
        Assert(
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_condition
        );
        virtual ~Assert() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Assert>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Group>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<const BoundNode::Expr::IBase> m_Condition{};
    };
}
