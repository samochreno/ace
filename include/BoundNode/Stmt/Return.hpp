#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt
{
    class Return :
        public std::enable_shared_from_this<BoundNode::Stmt::Return>,
        public virtual BoundNode::Stmt::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Return, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Return>
    {
    public:
        Return(
            const std::shared_ptr<Scope>& t_scope,
            const std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>& t_optExpr
        );
        virtual ~Return() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Return>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Return>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const BoundNode::Expr::IBase>> m_OptExpr{};
    };
}
