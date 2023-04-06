#pragma once

#include <iostream>
#include <ostream>
#include <termcolor/termcolor.hpp>

#define ACE_LOG_EMPTY() std::cout << "\n"
#define ACE_LOG_INFO(t_message) std::cout << termcolor::bright_grey << "<Info> " << termcolor::reset << t_message << "\n"
#define ACE_LOG_WARNING(t_message) std::cout << termcolor::yellow << "<Warning> " << termcolor::reset << t_message << "\n"
#define ACE_LOG_ERROR(t_message) std::cout << termcolor::red << "<Error> " << termcolor::reset << t_message << "\n"
#define ACE_LOG_FLUSH(t_message) std::cout << std::flush
