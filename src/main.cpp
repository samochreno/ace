#include "Application.hpp"

#include <vector>
#include <string>

auto main(const int argc, const char* argv[]) -> int
{
    std::vector<std::string_view> args{};
    for (size_t i = 1; i < argc; i++)
    {
        args.emplace_back(argv[i]);
    }

    Ace::Application::Main(args);
    return 0;
}
