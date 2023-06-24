#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Type/Base.hpp"
#include "BoundNode/Impl.hpp"
#include "BoundNode/Function.hpp"
#include "BoundNode/Variable/Normal/Static.hpp"
#include "Scope.hpp"
#include "Symbol/Module.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    class Module : 
        public std::enable_shared_from_this<BoundNode::Module>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Module>,
        public virtual BoundNode::ILowerable<BoundNode::Module>
    {
    public:
        Module(
            Symbol::Module* const t_symbol,
            const std::vector<std::shared_ptr<const BoundNode::Module>>& t_modules,
            const std::vector<std::shared_ptr<const BoundNode::Type::IBase>>& t_types,
            const std::vector<std::shared_ptr<const BoundNode::Impl>>& t_impls,
            const std::vector<std::shared_ptr<const BoundNode::Function>>& t_functions,
            const std::vector<std::shared_ptr<const BoundNode::Variable::Normal::Static>>& t_variables
        );
        virtual ~Module() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Module>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Module>> final;

    private:
        Symbol::Module* m_Symbol{};
        std::vector<std::shared_ptr<const BoundNode::Module>> m_Modules{};
        std::vector<std::shared_ptr<const BoundNode::Type::IBase>> m_Types{};
        std::vector<std::shared_ptr<const BoundNode::Impl>> m_Impls{};
        std::vector<std::shared_ptr<const BoundNode::Function>> m_Functions{};
        std::vector<std::shared_ptr<const BoundNode::Variable::Normal::Static>> m_Variables{};
    };
}
