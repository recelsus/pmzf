#include "test_support.hpp"

#include <array>
#include <iostream>

void run_cli_tests();
void run_pubmed_cli_tests();
void run_pubmed_mapper_tests();
void run_version_tests();

int main()
{
    const std::array tests{
        TestCase{"cli", run_cli_tests},
        TestCase{"pubmed_cli", run_pubmed_cli_tests},
        TestCase{"pubmed_mapper", run_pubmed_mapper_tests},
        TestCase{"version", run_version_tests},
    };

    for (const auto& test : tests) {
        test.run();
        std::cout << "ok: " << test.name << '\n';
    }

    return 0;
}
