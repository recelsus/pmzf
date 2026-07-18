#pragma once

#include "pmzf/app/cli.hpp"
#include "pmzf/pubmed/options.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace pmzf::pubmed {

struct CliResult {
    std::optional<Options> value;
    std::string error;
};

[[nodiscard]] CliResult parse_options(const std::vector<std::string>& args);
[[nodiscard]] std::span<const std::string_view> value_option_names();
[[nodiscard]] std::span<const std::string_view> flag_option_names();
[[nodiscard]] std::string usage();

} // namespace pmzf::pubmed
