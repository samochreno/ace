#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbol/Var/Normal/Static.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Var::Normal
{
    class Static :
        public std::enable_shared_from_this<BoundNode::Var::Normal::Static>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<Symbol::Var::Normal::Static>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Var::Normal::Static>,
        public virtual BoundNode::ILowerable<BoundNode::Var::Normal::Static>
    {
    public:
        Static(
            Symbol::Var::Normal::Static* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes
        );
        virtual ~Static() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Var::Normal::Static>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Var::Normal::Static>> final;

        auto GetSymbol() const -> Symbol::Var::Normal::Static* final;

    private:
        Symbol::Var::Normal::Static* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
    };
}
