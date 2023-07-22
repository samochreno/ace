#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

#include "SrcLocation.hpp"
#include "Compilation.hpp"
#include "Scope.hpp"
#include "SymbolCreatable.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"

namespace Ace
{
    class INode
    {
    public:
        virtual ~INode() = default;

        virtual auto GetSrcLocation() const -> const SrcLocation& = 0;
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
            const std::shared_ptr<Scope>& scope
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
        std::vector<const INode*>& vec,
        const T* const node
    ) -> void
    {
        ACE_ASSERT(node);

        vec.push_back(node);
        std::vector<const INode*> childChildren = node->GetChildren();
        vec.insert(end(vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& vec,
        const std::shared_ptr<const T>& node
    ) -> void
    {
        AddChildren(vec, node.get());
    }

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& vec,
        const std::vector<const T*>& nodes
    ) -> void
    {
        std::for_each(begin(nodes), end(nodes), 
        [&](const T* const node)
        {
            AddChildren(vec, node);
        });
    }

    template<typename T>
    auto AddChildren(
        std::vector<const INode*>& vec,
        const std::vector<std::shared_ptr<const T>>& nodes
    ) -> void
    {
        std::for_each(begin(nodes), end(nodes), 
        [&](const std::shared_ptr<const T>& node)
        {
            AddChildren(vec, node);
        });
    }
}
