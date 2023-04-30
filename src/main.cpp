#include "Application.hpp"

#include <vector>
#include <string>

auto main(int t_argc, char* t_argv[]) -> int
{
    std::vector<std::string> args{};
    for (size_t i = 1; i < t_argc; i++)
    {
        args.emplace_back(t_argv[i]);
    }

    Ace::Main(args);
    return 0;
}
