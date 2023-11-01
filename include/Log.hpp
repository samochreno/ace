#pragma once

#include <ostream>

namespace Ace
{
    struct LoggerConfiguration
    {
        LoggerConfiguration(
            std::ostream& stream
        ) : Stream{ stream }
        {
        }

        std::ostream& Stream;
    };

    class Logger
    {
    public:
        Logger(
            const LoggerConfiguration configuration
        ) : m_Configuration{ configuration }
        {
        }

        template<typename T>
        auto operator<<(const T& value) const -> const Logger&
        {
            m_Configuration.Stream << value;
            return *this;
        }
        auto operator<<(std::ostream&(*function)(std::ostream&)) const -> const Logger&
        {
            function(m_Configuration.Stream);
            return *this;
        }

    protected:
        LoggerConfiguration m_Configuration;
    };

    extern Logger Log;
    extern Logger LogDebug;
}
