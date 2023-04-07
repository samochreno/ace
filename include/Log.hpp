#pragma once

#include <iostream>
#include <ostream>
#include <termcolor/termcolor.hpp>

namespace Ace::Log
{
    auto CreateTimestamp() -> std::string;
}

#define ACE_LOG_COLOR_WHITE       255, 255, 255
#define ACE_LOG_COLOR_TIMESTAMP    44,  62,  80
#define ACE_LOG_COLOR_INFORMATION  38, 162, 105
#define ACE_LOG_COLOR_WARNING     243, 156,  18
#define ACE_LOG_COLOR_ERROR       192,  57,  43

#define ACE_LOG_EMPTY()          std::cout << "\n"
#define ACE_LOG_FLUSH(t_message) std::cout << std::flush

#define ACE_LOG_INFO(t_message) \
    std::cout << termcolor::color<ACE_LOG_COLOR_WHITE> << termcolor::on_color<ACE_LOG_COLOR_INFORMATION> << \
    " | INFORMATION   " << termcolor::on_color<ACE_LOG_COLOR_TIMESTAMP> << \
    " " << Ace::Log::CreateTimestamp() << " " << termcolor::on_color<ACE_LOG_COLOR_INFORMATION> << \
    " " << termcolor::reset << termcolor::color<ACE_LOG_COLOR_INFORMATION> << \
    " " << t_message << "\n"

#define ACE_LOG_WARNING(t_message) \
    std::cout << termcolor::color<ACE_LOG_COLOR_WHITE> << termcolor::on_color<ACE_LOG_COLOR_WARNING> << \
    " | WARNING       " << termcolor::on_color<ACE_LOG_COLOR_TIMESTAMP> << \
    " " << Ace::Log::CreateTimestamp() << " " << termcolor::on_color<ACE_LOG_COLOR_WARNING> << \
    " " << termcolor::reset << termcolor::color<ACE_LOG_COLOR_WARNING> << \
    " " << t_message << "\n"

#define ACE_LOG_ERROR(t_message) \
    std::cout << termcolor::color<ACE_LOG_COLOR_WHITE> << termcolor::on_color<ACE_LOG_COLOR_ERROR> << \
    " | ERROR         " << termcolor::on_color<ACE_LOG_COLOR_TIMESTAMP> << \
    " " << Ace::Log::CreateTimestamp() << " " << termcolor::on_color<ACE_LOG_COLOR_ERROR> << \
    " " << termcolor::reset << termcolor::color<ACE_LOG_COLOR_ERROR> << \
    " " << t_message << "\n"
