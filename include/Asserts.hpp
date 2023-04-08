#pragma once

#include <exception>

#include "Log.hpp"

#define ACE_ASSERT(t_condition) \
    if (!(t_condition)) \
    { \
        ACE_LOG_ERROR( \
            "Assertion failed in file " << __FILE__ << \
            " at line " << __LINE__ << ": " << #t_condition \
        ); \
        ACE_LOG_FLUSH(); \
        std::terminate(); \
    } static_assert(true, "Semicolon required.")

#define ACE_UNREACHABLE() \
    ACE_LOG_ERROR( \
        "Reached unreachable location in file " << __FILE__ << \
        " at line " << __LINE__ \
    ); \
    ACE_LOG_FLUSH(); \
    std::terminate()
