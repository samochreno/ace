#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbol/Variable/Parameter/Normal.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Variable::Parameter
{
    class Normal : 
        public std::enable_shared_from_this<BoundNode::Variable::Parameter::Normal>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<Symbol::Variable::Parameter::Normal>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Variable::Parameter::Normal>,
        public virtual BoundNode::ILowerable<BoundNode::Variable::Parameter::Normal>
    {
    public:
        Normal(
            Symbol::Variable::Parameter::Normal* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes
        ) : m_Symbol{ t_symbol },
            m_Attributes{ t_attributes }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Symbol->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Normal>>> final;
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Normal>>> final;

        auto GetSymbol() const -> Symbol::Variable::Parameter::Normal* final { return m_Symbol; }

    private:
        Symbol::Variable::Parameter::Normal* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
    };
}
