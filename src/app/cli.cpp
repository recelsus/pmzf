#include "pmzf/app/cli.hpp"

#include "spagyrist/spagyrist.hpp"

#include <utility>

namespace pmzf::app {
namespace {

bool contains(std::span<const std::string_view> values, std::string_view target)
{
    for (const auto value : values) {
        if (value == target) {
            return true;
        }
    }
    return false;
}

ParseResult success(Command value)
{
    ParseResult result;
    result.value = std::move(value);
    return result;
}

ParseResult failure(std::string error)
{
    ParseResult result;
    result.error = std::move(error);
    return result;
}

} // namespace

std::string usage()
{
    return "Usage: pmzf [options] <query>\n"
           "\n"
           "Client options:\n"
           "  -h, --help                 Show this help.\n";
}

ParseResult parse_args(
    const std::vector<std::string>& args,
    std::span<const std::string_view> external_value_options,
    std::span<const std::string_view> external_flag_options)
{
    Command parsed;
    std::vector<std::string> remaining_args;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];

        if (arg == "-h" || arg == "--help") {
            parsed.help = true;
            return success(parsed);
        }
        if (arg == "--version") {
            parsed.version = true;
            return success(parsed);
        }
        if (arg == "--info") {
            parsed.info = true;
            return success(parsed);
        }

        if (contains(external_value_options, arg)) {
            if (i + 1 >= args.size()) {
                return failure("missing value for " + arg);
            }
            remaining_args.push_back(arg);
            remaining_args.push_back(args[i + 1]);
            ++i;
            continue;
        }
        if (contains(external_flag_options, arg)) {
            remaining_args.push_back(arg);
            continue;
        }

        if (spagyrist::is_common_value_option(arg)) {
            if (i + 1 >= args.size()) {
                return failure("missing value for " + arg);
            }
            remaining_args.push_back(arg);
            remaining_args.push_back(args[i + 1]);
            ++i;
            continue;
        }
        if (spagyrist::is_common_flag_option(arg)) {
            remaining_args.push_back(arg);
            continue;
        }

        if (!arg.empty() && arg.front() == '-') {
            return failure("unknown option: " + arg);
        }

        if (!parsed.query.empty()) {
            return failure("multiple query arguments are not supported");
        }
        parsed.query = arg;
    }

    const auto spagyrist_parsed = spagyrist::parse_common_args(remaining_args);
    if (spagyrist_parsed.error) {
        return failure(*spagyrist_parsed.error);
    }
    parsed.app.spagyrist = spagyrist_parsed.options;

    if (!parsed.help && !parsed.version && !parsed.info && parsed.query.empty()) {
        return failure("query is required");
    }

    return success(parsed);
}

} // namespace pmzf::app
