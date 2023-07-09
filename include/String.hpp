#pragma once

#include <string>

namespace Ace
{
    auto IsInAlphabet(const char t_character) -> bool;
    auto IsNumber    (const char t_character) -> bool;

    auto TrimRight(std::string& t_value) -> std::string&;
    auto TrimLeft (std::string& t_value) -> std::string&;
    auto Trim     (std::string& t_value) -> std::string&;

    auto MakeLowercase(std::string& t_value) -> std::string&;
    auto MakeUppercase(std::string& t_value) -> std::string&;
}
