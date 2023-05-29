#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Function.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    class Impl : 
        public std::enable_shared_from_this<BoundNode::Impl>,
        public virtual BoundNode::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Impl>,
        public virtual BoundNode::ILowerable<BoundNode::Impl>
    {
    public:
        Impl(
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const BoundNode::Function>>& t_functions
        ) : m_Scope{ t_scope },
            m_Functions{ t_functions }
        {
        }
        virtual ~Impl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Impl>>> final;
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Impl>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const BoundNode::Function>> m_Functions{};
    };
}
