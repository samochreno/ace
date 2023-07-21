#include "String.hpp"

#include <cctype>

namespace Ace
{
    static auto IsLowercase(const char character) -> bool
    {
        return islower(character);
    }

    static auto IsUppercase(const char character) -> bool
    {
        return isupper(character);
    }

    auto IsInAlphabet(const char character) -> bool
    {
        return IsLowercase(character) || IsUppercase(character);
    }

    auto IsNumber(const char character) -> bool
    {
        return (character >= '0') && (character <= '9');
    }

    auto TrimRight(std::string& value) -> std::string&
    {
        value.erase(value.find_last_not_of(' ') + 1);
        return value;
    }

    auto TrimLeft(std::string& value) -> std::string&
    {
        value.erase(0, value.find_first_not_of(' '));
        return value;
    }

    auto Trim(std::string& value) -> std::string&
    {
        TrimLeft(value);
        TrimRight(value);
        return value;
    }

    auto MakeLowercase(std::string& value) -> std::string&
    {
        std::for_each(begin(value), end(value),
        [](char& value)
        {
            if (IsUppercase(value))
            {
                value = tolower(value);
            }
        });

        return value;
    }

    auto MakeUppercase(std::string& value) -> std::string&
    {
        std::for_each(begin(value), end(value),
        [](char& value)
        {
            if (IsLowercase(value))
            {
                value = toupper(value);
            }
        });

        return value;
    }
}
