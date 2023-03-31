#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Variable::Normal
{
    class Instance : 
        public std::enable_shared_from_this<BoundNode::Variable::Normal::Instance>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<Symbol::Variable::Normal::Instance>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Variable::Normal::Instance>,
        public virtual BoundNode::ILowerable<BoundNode::Variable::Normal::Instance>
    {
    public:
        Instance(
            Symbol::Variable::Normal::Instance* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes
        ) : m_Symbol{ t_symbol },
            m_Attributes{ t_attributes }
        {
        }
        virtual ~Instance() = default;

        auto GetScope() const -> Scope* final { return m_Symbol->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Normal::Instance>>> final;
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Normal::Instance>>> final;

        auto GetSymbol() const -> Symbol::Variable::Normal::Instance* final { return m_Symbol; }

    private:
        Symbol::Variable::Normal::Instance* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
    };
}
