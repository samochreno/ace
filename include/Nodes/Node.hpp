#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

#include "SourceLocation.hpp"
#include "Compilation.hpp"
#include "Scope.hpp"
#include "SymbolCreatable.hpp"
#include "Diagnostics.hpp"
#include "Assert.hpp"

namespace Ace
{
    class INode
    {
    public:
        virtual ~INode() = default;

        virtual auto GetSourceLocation() const -> const SourceLocation& = 0;
        virtual auto GetCompilation() const -> const Compilation* final;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetChildren() const -> std::vector<const INode*> = 0;
    };

    template<typename T>
    class ICloneableNode : public virtual INode
    {
    public:
        virtual ~ICloneableNode() = default;

        virtual auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const T> = 0;
    };

    template<typename T>
    class IBindableNode : public virtual INode
    {
    public:
        virtual ~IBindableNode() = default;

        virtual auto CreateBound() const -> Expected<std::shared_ptr<const T>> = 0;
    };

    class ISymbolCreatableNode :
        public virtual INode,
        public virtual Ace::ISymbolCreatable
    {
    public:
        virtual ~ISymbolCreatableNode() = default;
    };

    class IPartiallySymbolCreatableNode :
        public virtual ISymbolCreatableNode,
        public virtual Ace::IPartiallySymbolCreatable
    {
    public:
        virtual ~IPartiallySymbolCreatableNode() = default;
    };

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& t_vec,
        const T* const t_node
    ) -> void
    {
        ACE_ASSERT(t_node);

        t_vec.push_back(t_node);
        std::vector<const INode*> childChildren = t_node->GetChildren();
        t_vec.insert(end(t_vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& t_vec,
        const std::shared_ptr<const T>& t_node
    ) -> void
    {
        AddChildren(t_vec, t_node.get());
    }

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& t_vec,
        const std::vector<const T*>& t_nodes
    ) -> void
    {
        std::for_each(begin(t_nodes), end(t_nodes), 
        [&](const T* const t_node)
        {
            AddChildren(t_vec, t_node);
        });
    }

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& t_vec,
        const std::vector<std::shared_ptr<const T>>& t_nodes
    ) -> void
    {
        std::for_each(begin(t_nodes), end(t_nodes), 
        [&](const std::shared_ptr<const T>& t_node)
        {
            AddChildren(t_vec, t_node);
        });
    }
}
