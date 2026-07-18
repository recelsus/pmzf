#include "pmzf/pubmed/application.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc > 0 ? argc - 1 : 0));
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    return pmzf::pubmed::run(args, std::cout, std::cerr);
}
