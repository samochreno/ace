#include "Application.hpp"

#include <vector>
#include <string>

auto main(const int t_argc, const char* t_argv[]) -> int
{
    std::vector<std::string_view> args{};
    for (size_t i = 1; i < t_argc; i++)
    {
        args.emplace_back(t_argv[i]);
    }

    Ace::Main(args);
    return 0;
}
