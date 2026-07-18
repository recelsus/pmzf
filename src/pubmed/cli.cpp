#include "pmzf/pubmed/cli.hpp"

#include "spagyrist/spagyrist.hpp"

#include <charconv>
#include <string_view>
#include <utility>

namespace pmzf::pubmed {
namespace {

std::optional<std::size_t> parse_size(std::string_view value)
{
    std::size_t parsed{};
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }
    return parsed;
}

CliResult success(Options value)
{
    CliResult result;
    result.value = std::move(value);
    return result;
}

CliResult failure(std::string error)
{
    CliResult result;
    result.error = std::move(error);
    return result;
}

bool is_spagyrist_option(std::string_view arg)
{
    return arg == "--format" || arg == "-f"
        || arg == "--output" || arg == "-o"
        || arg == "--select" || arg == "-s";
}

bool is_general_option(std::string_view arg)
{
    return arg == "-h" || arg == "--help" || arg == "--version" || arg == "--info";
}

} // namespace

std::string usage()
{
    return "  -d, --date-range <range>  Apply a PubMed date range filter, e.g. 2024:2025[DP].\n"
           "  -r, --retmax <n>         Maximum results to fetch. Default: 20\n";
}

std::span<const std::string_view> value_option_names()
{
    static constexpr std::string_view names[]{
        "--date-range",
        "-d",
        "--retmax",
        "-r",
        "--limit",
    };
    return names;
}

std::span<const std::string_view> flag_option_names()
{
    return {};
}

CliResult parse_options(const std::vector<std::string>& args)
{
    Options parsed;
    parsed.config = load_client_config();

    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];

        if (is_general_option(arg)) {
            continue;
        }
        if (is_spagyrist_option(arg)) {
            if (i + 1 >= args.size()) {
                return failure("missing value for " + arg);
            }
            ++i;
            continue;
        }

        if (arg == "--date-range" || arg == "-d") {
            if (i + 1 >= args.size()) {
                return failure("missing value for " + arg);
            }
            parsed.date_range = args[++i];
            continue;
        }
        if (arg == "--retmax" || arg == "-r" || arg == "--limit") {
            if (i + 1 >= args.size()) {
                return failure("missing value for " + arg);
            }
            const auto limit = parse_size(args[++i]);
            if (!limit || *limit == 0) {
                return failure("invalid retmax: " + args[i]);
            }
            parsed.limit = *limit;
            continue;
        }

        if (!arg.empty() && arg.front() == '-') {
            return failure("unknown option: " + arg);
        }
    }

    return success(parsed);
}

} // namespace pmzf::pubmed
