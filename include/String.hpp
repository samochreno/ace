#pragma once

#include <string>

namespace Ace
{
    auto IsInAlphabet(const char character) -> bool;
    auto IsNumber    (const char character) -> bool;

    auto TrimRight(std::string& value) -> std::string&;
    auto TrimLeft (std::string& value) -> std::string&;
    auto Trim     (std::string& value) -> std::string&;

    auto MakeLowercase(std::string& value) -> std::string&;
    auto MakeUppercase(std::string& value) -> std::string&;
}
