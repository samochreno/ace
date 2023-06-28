#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Type/Base.hpp"
#include "BoundNode/Attribute.hpp"
#include "Nodes/Vars/InstanceVarNode.hpp"
#include "Scope.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

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
            StructTypeSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes,
            const std::vector<std::shared_ptr<const BoundNode::Var::Normal::Instance>>& t_variables
        );
        virtual ~Struct() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::Struct>>> final;
        auto GetOrCreateTypeCheckedType(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::Struct>> final;
        auto GetOrCreateLoweredType(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>> final;

        auto GetSymbol() const -> StructTypeSymbol* final;

    private:
        StructTypeSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
        std::vector<std::shared_ptr<const BoundNode::Var::Normal::Instance>> m_Vars{};
    };
}
