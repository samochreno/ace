#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Var::Normal
{
    class Instance : 
        public std::enable_shared_from_this<BoundNode::Var::Normal::Instance>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<InstanceVarSymbol>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Var::Normal::Instance>,
        public virtual BoundNode::ILowerable<BoundNode::Var::Normal::Instance>
    {
    public:
        Instance(
            InstanceVarSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes
        );
        virtual ~Instance() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Var::Normal::Instance>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Var::Normal::Instance>> final;

        auto GetSymbol() const -> InstanceVarSymbol* final;

    private:
        InstanceVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
    };
}
