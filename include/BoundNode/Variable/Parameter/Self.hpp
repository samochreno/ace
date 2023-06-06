#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbol/Variable/Parameter/Self.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Variable::Parameter
{
    class Self : 
        public std::enable_shared_from_this<BoundNode::Variable::Parameter::Self>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<Symbol::Variable::Parameter::Self>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Variable::Parameter::Self>,
        public virtual BoundNode::ILowerable<BoundNode::Variable::Parameter::Self>
    {
    public:
        Self(Symbol::Variable::Parameter::Self* const t_symbol)
            : m_Symbol{ t_symbol }
        {
        }
        virtual ~Self() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Symbol->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Self>>> final;
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Self>> final;

        auto GetSymbol() const -> Symbol::Variable::Parameter::Self* final { return m_Symbol; }

    private:
        Symbol::Variable::Parameter::Self* m_Symbol{};
    };
}
