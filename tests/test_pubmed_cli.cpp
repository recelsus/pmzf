#include "pmzf/pubmed/cli.hpp"

#include "test_support.hpp"

#include <string>
#include <vector>

namespace {

void parses_pubmed_options()
{
    const auto result = pmzf::pubmed::parse_options(
        std::vector<std::string>{"--date-range", "2024:2025[DP]", "--retmax", "5", "cancer"});

    PMZF_CHECK(result.value.has_value());
    PMZF_CHECK(result.value->date_range == "2024:2025[DP]");
    PMZF_CHECK(result.value->limit == 5);
}

void rejects_invalid_retmax()
{
    const auto result = pmzf::pubmed::parse_options(std::vector<std::string>{"--retmax", "0", "cancer"});

    PMZF_CHECK(!result.value.has_value());
    PMZF_CHECK(result.error == "invalid retmax: 0");
}

void reports_value_options_to_app_parser()
{
    const auto result = pmzf::app::parse_args(
        std::vector<std::string>{"--retmax", "5", "cancer"},
        pmzf::pubmed::value_option_names(),
        pmzf::pubmed::flag_option_names());

    PMZF_CHECK(result.value.has_value());
    PMZF_CHECK(result.value->query == "cancer");
}

} // namespace

void run_pubmed_cli_tests()
{
    parses_pubmed_options();
    rejects_invalid_retmax();
    reports_value_options_to_app_parser();
}
