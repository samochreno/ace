#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>

#include "Compilation.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

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

        virtual auto GetCompilation() const -> Compilation* final;
        virtual auto GetSrcLocation() const -> const SrcLocation& = 0;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto CollectChildren() const -> std::vector<const IBoundNode*> = 0;
    };

    template<typename TNode, typename TContext = TypeCheckingContext>
    class ITypeCheckableBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypeCheckableBoundNode() = default;

        virtual auto CreateTypeChecked(
            const TContext& context
        ) const -> Diagnosed<std::shared_ptr<const TNode>> = 0;
    };

    template<typename TNode, typename TContext = LoweringContext>
    class ILowerableBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ILowerableBoundNode() = default;

        virtual auto CreateLowered(
            const TContext& context
        ) const -> std::shared_ptr<const TNode> = 0;
    };

    template<typename T>
    auto AddChildren(
        std::vector<const IBoundNode*>& vec,
        const std::shared_ptr<const T>& node
    ) -> void
    {
        if (!node)
        {
            return;
        }

        vec.push_back(node.get());
        std::vector<const IBoundNode*> childChildren = node->CollectChildren();
        vec.insert(end(vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(
        std::vector<const IBoundNode*>& vec,
        const std::vector<std::shared_ptr<const T>>& nodes
    ) -> void
    {
        std::for_each(begin(nodes), end(nodes),
        [&](const std::shared_ptr<const T>& node)
        {
            vec.push_back(node.get());

            auto children = node->CollectChildren();
            vec.insert(end(vec), begin(children), end(children));
        });
    }
}
