#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "BoundNode/Impl.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node
{
    class Impl :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::Impl>,
        public virtual Node::IBindable<BoundNode::Impl>
    {
    public:
        Impl(
            const std::shared_ptr<Scope>& t_selfScope,
            const SymbolName& t_typeName,
            const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
            const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates
        );
        virtual ~Impl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Impl> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Impl>> final;

        auto DefineAssociations() const -> Expected<void>;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        SymbolName m_TypeName{};
        std::vector<std::shared_ptr<const Node::Function>> m_Functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> m_FunctionTemplates{};
    };
}
