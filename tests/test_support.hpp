#pragma once

#include <cstdlib>
#include <iostream>
#include <string_view>

struct TestCase {
    std::string_view name;
    void (*run)();
};

#define PMZF_CHECK(condition)                                                                        \
    do {                                                                                             \
        if (!(condition)) {                                                                          \
            std::cerr << "check failed: " << #condition << " at " << __FILE__ << ':' << __LINE__     \
                      << '\n';                                                                       \
            std::exit(1);                                                                            \
        }                                                                                            \
    } while (false)
