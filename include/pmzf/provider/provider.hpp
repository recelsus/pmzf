#pragma once

#include "spagyrist/spagyrist.hpp"

#include <string>
#include <vector>

namespace pmzf::provider {

class Provider {
public:
    virtual ~Provider() = default;

    virtual std::vector<spagyrist::candidate>
    search(const std::string& query) = 0;

    virtual spagyrist::document
    fetch(const spagyrist::selection& selected) = 0;
};

} // namespace pmzf::provider
