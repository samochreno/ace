#pragma once

#include <vector>
#include <optional>
#include <type_traits>

namespace Ace
{
    template<typename T>
    struct Cacheable 
    {
        Cacheable(
        ) : IsChanged{ true }
        {
        }

        Cacheable(
            bool isChanged,
            const T& value
        ) : IsChanged{ isChanged },
            Value{ value }
        {
        }

        template<typename TNew>
        operator Cacheable<TNew>() const
        {
            return Cacheable<TNew>(IsChanged, Value);
        }

        bool IsChanged{};
        T Value{};
    };

#define TOut typename std::decay_t<decltype(func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto CreateChanged(const TIn& node, F&& func) -> Cacheable<TOut>
    {
        auto cchNode = func(node);
        return { true, cchNode.Value };
    }

#undef TOut

    template<typename T>
    auto CreateChanged(const T& node) -> Cacheable<T>
    {
        return { true, node };
    }

    template<typename T>
    auto CreateUnchanged(const T& node) -> Cacheable<T>
    {
        return { false, node };
    }

#define TOut typename std::decay_t<decltype(func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto TransformCacheableVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> Cacheable<std::vector<TOut>>
    {
        std::vector<TOut> vec{};
        bool isChanged = false;

        for (size_t i = 0; i < inVec.size(); i++)
        {
            auto cchLowered = func(inVec.at(i));
            if (!cchLowered.IsChanged)
            {
                continue;
            }

            if (!isChanged)
            {
                isChanged = true;
                vec = inVec;
            }

            vec.at(i) = cchLowered.Value;
        }

        if (!isChanged)
        {
            return CreateUnchanged(inVec);
        }

        return CreateChanged(vec);
    }

#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto TransformCacheableOptional(
        const std::optional<TIn>& optNode,
        F&& func
    ) -> Cacheable<std::optional<TOut>>
    {
        if (!optNode)
        {
            return CreateUnchanged(std::optional<TOut>{});
        }

        auto cchNode = func(optNode.value());
        return { cchNode.IsChanged, std::optional<TOut>{ cchNode.Value } };
    }

#undef TOut
}
