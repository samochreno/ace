#include "String.hpp"

#include <cctype>

namespace Ace
{
    static auto IsLowercase(const char t_character) -> bool
    {
        return islower(t_character);
    }

    static auto IsUppercase(const char t_character) -> bool
    {
        return isupper(t_character);
    }

    auto IsInAlphabet(const char t_character) -> bool
    {
        return IsLowercase(t_character) || IsUppercase(t_character);
    }

    auto IsNumber(const char t_character) -> bool
    {
        return (t_character >= '0') && (t_character <= '9');
    }

    auto TrimRight(std::string& t_value) -> std::string&
    {
        t_value.erase(t_value.find_last_not_of(' ') + 1);
        return t_value;
    }

    auto TrimLeft(std::string& t_value) -> std::string&
    {
        t_value.erase(0, t_value.find_first_not_of(' '));
        return t_value;
    }

    auto Trim(std::string& t_value) -> std::string&
    {
        TrimLeft(t_value);
        TrimRight(t_value);
        return t_value;
    }

    auto MakeLowercase(std::string& t_value) -> std::string&
    {
        std::for_each(begin(t_value), end(t_value),
        [](char& t_value)
        {
            if (IsUppercase(t_value))
            {
                t_value = tolower(t_value);
            }
        });

        return t_value;
    }

    auto MakeUppercase(std::string& t_value) -> std::string&
    {
        std::for_each(begin(t_value), end(t_value),
        [](char& t_value)
        {
            if (IsLowercase(t_value))
            {
                t_value = toupper(t_value);
            }
        });

        return t_value;
    }
}
