#include "Utility.hpp"

namespace Ace
{
    auto IsInAlphabet(const char& t_character) -> bool
    {
        return
            ((t_character >= 'a') && (t_character <= 'z')) ||
            ((t_character >= 'A') && (t_character <= 'Z'));
    }

    auto IsNumber(const char& t_character) -> bool
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
}
