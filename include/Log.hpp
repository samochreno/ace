#pragma once

#include <ostream>

namespace Ace
{
    struct LoggerConfiguration
    {
        LoggerConfiguration(
            std::ostream& t_stream
        ) : Stream{ t_stream }
        {
        }

        std::ostream& Stream;
    };

    class Logger
    {
    public:
        Logger(
            const LoggerConfiguration t_configuration
        ) : m_Configuration{ t_configuration }
        {
        }

        template<typename T>
        auto operator<<(const T& t_value) const -> const Logger&
        {
            m_Configuration.Stream << t_value;
            return *this;
        }
        auto operator<<(std::ostream& (*t_func)(std::ostream&)) const -> const Logger&
        {
            t_func(m_Configuration.Stream);
            return *this;
        }

    protected:
        LoggerConfiguration m_Configuration;
    };

    extern Logger Log;
    extern Logger LogDebug;
}
