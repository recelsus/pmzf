#include "pmzf/app/cli.hpp"

#include "test_support.hpp"

#include <string_view>
#include <string>
#include <vector>

namespace {

void parses_query()
{
    const auto result = pmzf::app::parse_args(std::vector<std::string>{"cancer"});

    PMZF_CHECK(result.value.has_value());
    PMZF_CHECK(result.value->query == "cancer");
    PMZF_CHECK(!result.value->help);
}

void reports_missing_query()
{
    const auto result = pmzf::app::parse_args({});

    PMZF_CHECK(!result.value.has_value());
    PMZF_CHECK(result.error == "query is required");
}

void reports_unknown_option()
{
    const auto result = pmzf::app::parse_args(std::vector<std::string>{"--unknown"});

    PMZF_CHECK(!result.value.has_value());
    PMZF_CHECK(result.error == "unknown option: --unknown");
}

void parses_help_without_query()
{
    const auto result = pmzf::app::parse_args(std::vector<std::string>{"--help"});

    PMZF_CHECK(result.value.has_value());
    PMZF_CHECK(result.value->help);
}

void client_usage_omits_spagyrist_options()
{
    const auto usage = pmzf::app::usage();

    PMZF_CHECK(usage.find("-h, --help") != std::string_view::npos);
    PMZF_CHECK(usage.find("--version") == std::string_view::npos);
    PMZF_CHECK(usage.find("--info") == std::string_view::npos);
}

} // namespace

void run_cli_tests()
{
    parses_query();
    reports_missing_query();
    reports_unknown_option();
    parses_help_without_query();
    client_usage_omits_spagyrist_options();
}
