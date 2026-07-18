#pragma once

#include "pmzf/app/options.hpp"
#include "pmzf/provider/provider.hpp"

#include <iosfwd>
#include <string>

namespace pmzf::app {

int run(
    const std::string& query,
    const Options& options,
    provider::Provider& provider,
    std::ostream& out,
    std::ostream& err);

} // namespace pmzf::app
