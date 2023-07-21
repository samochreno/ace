#pragma once

#include <type_traits>

namespace Ace
{
    template<typename T>
    struct MaybeChanged
    {
        MaybeChanged()
            : IsChanged{ true }
        {
        }

        MaybeChanged(bool isChanged, const T& value)
            : IsChanged{ isChanged }, Value{ value }
        {
        }

        template<typename TNew>
        operator MaybeChanged<TNew>() const
        {
            return MaybeChanged<TNew>(IsChanged, Value);
        }

        bool IsChanged{};
        T Value{};
    };

#define TOut typename std::decay_t<decltype(func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto CreateChanged(const TIn& node, F&& func) -> MaybeChanged<TOut>
    {
        auto mchNode = func(node);
        return { true, mchNode.Value };
    }

#undef TOut

    template<typename T>
    auto CreateChanged(const T& node) -> MaybeChanged<T>
    {
        return { true, node };
    }

    template<typename T>
    auto CreateUnchanged(const T& node) -> MaybeChanged<T>
    {
        return { false, node };
    }

#define TOut typename std::decay_t<decltype(func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto TransformMaybeChangedVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> MaybeChanged<std::vector<TOut>>
    {
        std::vector<TOut> vec{};
        bool isChanged = false;

        for (size_t i = 0; i < inVec.size(); i++)
        {
            auto mchLowered = func(inVec.at(i));
            if (!mchLowered.IsChanged)
            {
                continue;
            }

            if (!isChanged)
            {
                isChanged = true;
                vec = inVec;
            }

            vec.at(i) = mchLowered.Value;
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
    auto TransformMaybeChangedOptional(
        const std::optional<TIn>& optNode,
        F&& func
    ) -> MaybeChanged<std::optional<TOut>>
    {
        if (!optNode)
        {
            return CreateUnchanged(std::optional<TOut>{});
        }

        auto mchNode = func(optNode.value());
        return { mchNode.IsChanged, std::optional<TOut>{ mchNode.Value } };
    }

#undef TOut
}
