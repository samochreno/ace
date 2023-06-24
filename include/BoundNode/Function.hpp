#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Attribute.hpp"
#include "BoundNode/Variable/Parameter/Self.hpp"
#include "BoundNode/Variable/Parameter/Normal.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "Symbol/Function.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    class Function : 
        public std::enable_shared_from_this<BoundNode::Function>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITyped<Symbol::Function>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Function>,
        public virtual BoundNode::ILowerable<BoundNode::Function>
    {
    public:
        Function(
            Symbol::Function* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes,
            const std::optional<const std::shared_ptr<const BoundNode::Variable::Parameter::Self>>& t_optSelf,
            const std::vector<std::shared_ptr<const BoundNode::Variable::Parameter::Normal>>& t_parameters,
            const std::optional<std::shared_ptr<const BoundNode::Statement::Block>>& t_optBody
        );
        virtual ~Function() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Function>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Function>> final;

        auto GetSymbol() const -> Symbol::Function* final;

        auto GetBody() const -> std::optional<std::shared_ptr<const BoundNode::Statement::Block>>;

    private:
        Symbol::Function* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Attribute>> m_Attributes{};
        std::optional<std::shared_ptr<const BoundNode::Variable::Parameter::Self>> m_OptSelf{};
        std::vector<std::shared_ptr<const BoundNode::Variable::Parameter::Normal>> m_Parameters{};
        std::optional<std::shared_ptr<const BoundNode::Statement::Block>> m_OptBody{};
    };
}
