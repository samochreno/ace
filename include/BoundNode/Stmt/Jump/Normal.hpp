#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Jump/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Label.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt::Jump
{
    class Normal :
        public std::enable_shared_from_this<BoundNode::Stmt::Jump::Normal>,
        public virtual BoundNode::Stmt::Jump::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Jump::Normal, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Jump::Normal>
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Label* const t_labelSymbol
        );
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Jump::Normal>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Jump::Normal>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> Symbol::Label* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Label* m_LabelSymbol{};
    };
}
