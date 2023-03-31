#pragma once

#include <cassert>
#include <exception>

#define ACE_ASSERT(t_condition) assert(t_condition)
#define ACE_UNREACHABLE() assert(false); std::terminate()
