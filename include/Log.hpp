#pragma once

#include <iostream>

#define ACE_LOG_EMPTY() std::cout << "\n"
#define ACE_LOG_INFO(t_message) std::cout << "\033[90m<Info>\033[37m " << t_message << "\033[37m\n"
#define ACE_LOG_WARNING(t_message) std::cout << "\033[33m<Warning>\033[93m " << t_message << "\033[37m\n"
