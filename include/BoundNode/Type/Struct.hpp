#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Type/Base.hpp"
#include "BoundNode/Attribute.hpp"
#include "Node/Variable/Normal/Instance.hpp"
#include "Scope.hpp"
#include "Symbol/Type/Struct.hpp"

namespace Ace::BoundNode::Type
{
    class Struct : 
        public std::enable_shared_from_this<BoundNode::Type::Struct>,
        public virtual BoundNode::Type::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Type::Struct>,
        public virtual BoundNode::ILowerable<BoundNode::Type::Struct>
    {
    public:
        Struct(
            Symbol::Type::Struct* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes,
            const std::vector<std::shared_ptr<const BoundNode::Variable::Normal::Instance>>& t_variables
        ) : m_Symbol{ t_symbol },
            m_Attributes{ t_attributes },
            m_Variables{ t_variables }
        {
        }
        virtual ~Struct() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Symbol->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::Struct>>> final;
        auto GetOrCreateTypeCheckedType(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::Struct>> final;
        auto GetOrCreateLoweredType(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>> final { return GetOrCreateLowered(t_context); }

        auto GetSymbol() const -> Symbol::Type::Struct* final { return m_Symbol; }

    private:
        Symbol::Type::Struct* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
        std::vector<std::shared_ptr<const BoundNode::Variable::Normal::Instance>> m_Variables{};
    };
}
