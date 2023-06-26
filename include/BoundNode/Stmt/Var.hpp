#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"

namespace Ace::BoundNode::Stmt
{
    class Var :
        public std::enable_shared_from_this<BoundNode::Stmt::Var>,
        public virtual BoundNode::Stmt::IBase,
        public virtual BoundNode::ITyped<LocalVarSymbol>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Var, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Var>
    {
    public:
        Var(
            LocalVarSymbol* const t_symbol,
            const std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>& t_optAssignedExpr
        );
        virtual ~Var() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Var>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Var>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetSymbol() const -> LocalVarSymbol* final;
        
    private:
        LocalVarSymbol* m_Symbol{};
        std::optional<std::shared_ptr<const BoundNode::Expr::IBase>> m_OptAssignedExpr{};
    };
}
