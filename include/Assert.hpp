#pragma once

#include <exception>
#include <termcolor/termcolor.hpp>

#include "Log.hpp"

#define ACE_ASSERT(t_condition) \
    if (!(t_condition)) \
    { \
        Log << termcolor::bright_red << "Assertion failed in file"; \
        Log << __FILE__ << " at line " << __LINE__ << ": " << #t_condition; \
        Log << "\n" << std::flush; \
        std::terminate(); \
    } static_assert(true, "Semicolon required.")

#define ACE_UNREACHABLE() \
    Log << termcolor::bright_red << "Reached unreachable location in file"; \
    Log << __FILE__ << " at line " << __LINE__ << "\n" << std::flush; \
    std::terminate()
