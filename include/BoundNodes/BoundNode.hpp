#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>

#include "Compilation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    struct TypeCheckingContext
    {
    };

    struct LoweringContext
    {
    };

    class IBoundNode
    {
    public:
        virtual ~IBoundNode() = default;

        virtual auto GetCompilation() const -> const Compilation* final;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetChildren() const -> std::vector<const IBoundNode*> = 0;
    };

    template<typename TNode, typename TContext = TypeCheckingContext>
    class ITypeCheckableBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypeCheckableBoundNode() = default;

        virtual auto GetOrCreateTypeChecked(
            const TContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const TNode>>> = 0;
    };

    template<typename TNode, typename TContext = LoweringContext>
    class ILowerableBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ILowerableBoundNode() = default;

        virtual auto GetOrCreateLowered(
            const TContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const TNode>> = 0;
    };

    template<typename T>
    auto AddChildren(
        std::vector<const IBoundNode*>& t_vec,
        const std::shared_ptr<const T>& t_node
    ) -> void
    {
        if (!t_node)
        {
            return;
        }

        t_vec.push_back(t_node.get());
        std::vector<const IBoundNode*> childChildren = t_node->GetChildren();
        t_vec.insert(end(t_vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(
        std::vector<const IBoundNode*>& t_vec,
        const std::vector<std::shared_ptr<const T>>& t_nodes
    ) -> void
    {
        std::for_each(begin(t_nodes), end(t_nodes),
        [&](const std::shared_ptr<const T>& t_node)
        {
            t_vec.push_back(t_node.get());

            auto children = t_node->GetChildren();
            t_vec.insert(end(t_vec), begin(children), end(children));
        });
    }
}
