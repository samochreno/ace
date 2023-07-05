#pragma once

#include <string>
#include <exception>
#include <termcolor/termcolor.hpp>

#include "Log.hpp"

#define ACE_ASSERT(t_condition) \
    if (!(t_condition)) \
    { \
        Log( \
            DiagnosticSeverity::Debug, \
            std::string{} + \
            "Assertion failed in file " + __FILE__ + " at line " + \
            std::to_string(__LINE__) + ": " + #t_condition \
        ); \
        LogFlush(); \
        std::terminate(); \
    } static_assert(true, "Semicolon required.")

#define ACE_UNREACHABLE() \
    Log( \
        DiagnosticSeverity::Debug, \
        std::string{} + \
        "Reached unreachable location in file " + \
        __FILE__ + " at line " + std::to_string(__LINE__) \
    ); \
    LogFlush(); \
    std::terminate()
