#pragma once

#include <exception>
#include <termcolor/termcolor.hpp>

#include "Log.hpp"

#define ACE_ASSERT(condition) \
    if (!(condition)) \
    { \
        Out << termcolor::bright_red << "Assertion failed in file"; \
        Out << __FILE__ << " at line " << __LINE__ << ": " << #condition; \
        Out << "\n" << std::flush; \
        std::terminate(); \
    } static_assert(true, "Semicolon required.")

#define ACE_UNREACHABLE() \
    Out << termcolor::bright_red << "Reached unreachable location in file"; \
    Out << __FILE__ << " at line " << __LINE__ << "\n" << std::flush; \
    std::terminate()
