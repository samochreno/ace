#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>

#include "Compilation.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    struct Context
    {
        Context() = delete;

        struct TypeChecking
        {
            TypeChecking() = default;
        };

        struct Lowering
        {
            Lowering() = default;
        };
    };

    class IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetCompilation() const -> const Compilation&
        {
            return GetScope()->GetCompilation();
        }
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetChildren() const -> std::vector<const BoundNode::IBase*> = 0;
    };

    template<typename TNode, typename TContext = Context::TypeChecking>
    class ITypeCheckable
    {
    public:
        virtual ~ITypeCheckable() = default;

        virtual auto GetOrCreateTypeChecked(const TContext& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const TNode>>> = 0;
    };

    template<typename TNode, typename TContext = Context::Lowering>
    class ILowerable
    {
    public:
        virtual ~ILowerable() = default;

        virtual auto GetOrCreateLowered(const TContext& t_context) const -> MaybeChanged<std::shared_ptr<const TNode>> = 0;
    };

    template<typename T>
    auto AddChildren(std::vector<const BoundNode::IBase*>& t_vec, const std::shared_ptr<const T>& t_node) -> void
    {
        if (!t_node)
            return;

        t_vec.push_back(t_node.get());
        std::vector<const BoundNode::IBase*> childChildren = t_node->GetChildren();
        t_vec.insert(end(t_vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(std::vector<const BoundNode::IBase*>& t_vec, const std::vector<std::shared_ptr<const T>>& t_nodes) -> void
    {
        std::for_each(begin(t_nodes), end(t_nodes), [&](const std::shared_ptr<const T>& t_node)
        {
            t_vec.push_back(t_node.get());

            auto children = t_node->GetChildren();
            t_vec.insert(end(t_vec), begin(children), end(children));
        });
    }
}
