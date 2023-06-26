#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Var::Param
{
    class Self : 
        public std::enable_shared_from_this<BoundNode::Var::Param::Self>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<SelfParamVarSymbol>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Var::Param::Self>,
        public virtual BoundNode::ILowerable<BoundNode::Var::Param::Self>
    {
    public:
        Self(SelfParamVarSymbol* const t_symbol);
        virtual ~Self() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Var::Param::Self>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Var::Param::Self>> final;

        auto GetSymbol() const -> SelfParamVarSymbol* final;

    private:
        SelfParamVarSymbol* m_Symbol{};
    };
}
