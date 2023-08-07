#pragma once

#include <optional>
#include <functional>

namespace Ace
{
    template<typename T>
    class Lazy
    {
    public:
        Lazy(std::function<T()> initializer)
            : m_Initializer(std::move(initializer))
        {
        }

        auto Get() const -> const T&
        {
            if (!m_OptValue.has_value())
            {
                m_OptValue = m_Initializer();
            }

            return *m_OptValue;
        }

    private:
        std::function<T()> m_Initializer;
        mutable std::optional<T> m_OptValue{};
    };
}
