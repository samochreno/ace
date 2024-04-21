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
            auto overflow(int c) -> int
            {
               return c;
            }
        } m_Buffer;
    
    } NullStream{};

    Logger Out{ LoggerConfiguration{ std::cout } };
    Logger DebugOut
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
