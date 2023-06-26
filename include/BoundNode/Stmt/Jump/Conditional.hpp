#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Jump/Base.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt::Jump
{
    class Conditional :
        public std::enable_shared_from_this<BoundNode::Stmt::Jump::Conditional>,
        public virtual BoundNode::Stmt::Jump::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Jump::Conditional, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Jump::Conditional>
    {
    public:
        Conditional(
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_condition,
            LabelSymbol* const t_labelSymbol
        );
        virtual ~Conditional() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Jump::Conditional>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Jump::Conditional>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol* final;

    private:
        std::shared_ptr<const BoundNode::Expr::IBase> m_Condition{};
        LabelSymbol* m_LabelSymbol{};
    };
}
