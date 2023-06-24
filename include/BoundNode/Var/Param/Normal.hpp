#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbol/Var/Param/Normal.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Var::Param
{
    class Normal : 
        public std::enable_shared_from_this<BoundNode::Var::Param::Normal>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<Symbol::Var::Param::Normal>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Var::Param::Normal>,
        public virtual BoundNode::ILowerable<BoundNode::Var::Param::Normal>
    {
    public:
        Normal(
            Symbol::Var::Param::Normal* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes
        );
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Var::Param::Normal>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Var::Param::Normal>> final;

        auto GetSymbol() const -> Symbol::Var::Param::Normal* final;

    private:
        Symbol::Var::Param::Normal* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
    };
}
