#pragma once

#include "pmzf/app/options.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace pmzf::app {

struct Command {
    std::string query;
    Options app;
    bool help{false};
    bool version{false};
    bool info{false};
};

struct ParseResult {
    std::optional<Command> value;
    std::string error;
};

[[nodiscard]] ParseResult parse_args(
    const std::vector<std::string>& args,
    std::span<const std::string_view> external_value_options = {},
    std::span<const std::string_view> external_flag_options = {});
[[nodiscard]] std::string usage();

} // namespace pmzf::app
