#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "BoundNode/Impl.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"

namespace Ace::Node
{
    class Impl :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::Impl>,
        public virtual Node::IBindable<BoundNode::Impl>
    {
    public:
        Impl(
            Scope* const t_selfScope,
            const Name::Symbol::Full& t_typeName,
            const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
            const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates
        ) : m_SelfScope{ t_selfScope },
            m_TypeName{ t_typeName },
            m_Functions{ t_functions },
            m_FunctionTemplates{ t_functionTemplates }
        {
        }
        virtual ~Impl() = default;

        auto GetScope() const -> Scope* final { return m_SelfScope->GetParent(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Impl> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Impl>> final;

        auto DefineAssociations() const -> Expected<void>;

    private:
        Scope* m_SelfScope{};
        Name::Symbol::Full m_TypeName{};
        std::vector<std::shared_ptr<const Node::Function>> m_Functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> m_FunctionTemplates{};
    };
}
