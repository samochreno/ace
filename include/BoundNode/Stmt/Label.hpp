#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Base.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt
{
    class Label :
        public std::enable_shared_from_this<BoundNode::Stmt::Label>,
        public virtual BoundNode::Stmt::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Stmt::Label, BoundNode::Stmt::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Stmt::Label>
    {
    public:
        Label(LabelSymbol* const t_symbol);
        virtual ~Label() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Label>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Label>> final;
        auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol*;

    private:
        LabelSymbol* m_Symbol{};
    };
}
