#pragma once

namespace Ace
{
    template<typename T>
    struct MaybeChanged
    {
        MaybeChanged()
            : IsChanged{ true }
        {
        }

        MaybeChanged(bool t_isChanged, const T& t_value)
            : IsChanged{ t_isChanged }, Value{ t_value }
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

#define TOut typename std::decay_t<decltype(t_func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto CreateChanged(const TIn& t_node, F&& t_func) -> MaybeChanged<TOut>
    {
        auto mchNode = t_func(t_node);
        return { true, mchNode.Value };
    }

#undef TOut

    template<typename T>
    auto CreateChanged(const T& t_node) -> MaybeChanged<T>
    {
        return { true, t_node };
    }

    template<typename T>
    auto CreateUnchanged(const T& t_node) -> MaybeChanged<T>
    {
        return { false, t_node };
    }

#define TOut typename std::decay_t<decltype(t_func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto TransformMaybeChangedVector(const std::vector<TIn>& t_vec, F&& t_func) -> MaybeChanged<std::vector<TOut>>
    {
        std::vector<TOut> vec{};
        bool isChanged = false;

        for (size_t i = 0; i < t_vec.size(); i++)
        {
            auto mchLowered = t_func(t_vec[i]);
            if (!mchLowered.IsChanged)
                continue;

            if (!isChanged)
            {
                isChanged = true;
                vec = t_vec;
            }

            vec[i] = mchLowered.Value;
        }

        if (!isChanged)
            return CreateUnchanged(t_vec);

        return CreateChanged(vec);
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Value)>

    template<typename TIn, typename F>
    auto TransformMaybeChangedOptional(const std::optional<TIn>& t_optNode, F&& t_func) -> MaybeChanged<std::optional<TOut>>
    {
        if (!t_optNode)
            return CreateUnchanged(std::optional<TOut>{});

        auto mchNode = t_func(t_optNode.value());
        return { mchNode.IsChanged, std::optional<TOut>{ mchNode.Value } };
    }

#undef TOut
}
