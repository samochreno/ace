#pragma once

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

#include "Symbol/Creatable.hpp"
#include "Error.hpp"
#include "Scope.hpp"
#include "Asserts.hpp"

namespace Ace::Node
{
    class IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetScope() const -> Scope* = 0;
        virtual auto GetChildren() const -> std::vector<const IBase*> = 0;
    };

    template<typename T>
    class ICloneable
    {
    public:
        virtual ~ICloneable() = default;

        virtual auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const T> = 0;
    };

    template<typename T>
    class IBindable
    {
    public:
        virtual ~IBindable() = default;

        virtual auto CreateBound() const -> Expected<std::shared_ptr<const T>> = 0;
    };

    class ISymbolCreatable : public virtual Node::IBase, public virtual Symbol::ICreatable
    {
    public:
        virtual ~ISymbolCreatable() = default;
    };

    class IPartiallySymbolCreatable :
        public virtual Node::ISymbolCreatable,
        public virtual Symbol::IPartiallyCreatable
    {
    public:
        virtual ~IPartiallySymbolCreatable() = default;
    };

    template<typename T>
    auto AddChildren(std::vector<const Node::IBase*>& t_vec, const T* const t_node) -> void
    {
        ACE_ASSERT(t_node);

        t_vec.push_back(t_node);
        std::vector<const IBase*> childChildren = t_node->GetChildren();
        t_vec.insert(end(t_vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(std::vector<const Node::IBase*>& t_vec, const std::shared_ptr<const T>& t_node) -> void
    {
        AddChildren(t_vec, t_node.get());
    }

    template<typename T>
    auto AddChildren(std::vector<const Node::IBase*>& t_vec, const std::vector<const T*>& t_nodes) -> void
    {
        std::for_each(begin(t_nodes), end(t_nodes), [&](const T* const t_node)
        {
            AddChildren(t_vec, t_node);
        });
    }

    template<typename T>
    auto AddChildren(std::vector<const Node::IBase*>& t_vec, const std::vector<std::shared_ptr<const T>>& t_nodes) -> void
    {
        std::for_each(begin(t_nodes), end(t_nodes), [&](const std::shared_ptr<const T>& t_node)
        {
            AddChildren(t_vec, t_node);
        });
    }
}
