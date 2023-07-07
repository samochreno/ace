#include "Log.hpp"

#include <iostream>

namespace Ace
{
    static constexpr enum class LogLevelKind
    {
        Normal,
        Debug,
    } LogLevel = LogLevelKind::Normal;

    static class NullStream : public std::ostream
    {
    public:
        NullStream(
        ) : std::ostream{ &m_Buffer }
        {
        }

    private:
        class NullBuffer : public std::streambuf
        {
        public:
            auto overflow(int t_c) -> int
            {
               return t_c;
            }
        } m_Buffer;
    
    } NullStream{};

    Logger Log{ LoggerConfiguration{ std::cout } };
    Logger LogDebug
    {
        LoggerConfiguration
        {
            []() -> std::ostream&
            {
                switch (LogLevel)
                {
                    case LogLevelKind::Normal: return NullStream;
                    case LogLevelKind::Debug:  return std::cout;
                }
            }()
        }
    };

    
}
