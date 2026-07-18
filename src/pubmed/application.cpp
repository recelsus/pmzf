#include "pmzf/pubmed/application.hpp"

#include "pmzf/app/application.hpp"
#include "pmzf/app/version.hpp"
#include "pmzf/pubmed/cli.hpp"
#include "pmzf/pubmed/provider.hpp"
#include "spagyrist/spagyrist.hpp"

#include <ostream>

namespace pmzf::pubmed {

std::string application_usage()
{
    return app::usage() + pubmed::usage() + "\n" + spagyrist::cli_help_text();
}

int run(const std::vector<std::string>& args, std::ostream& out, std::ostream& err)
{
    const auto app_parsed = app::parse_args(args, value_option_names(), flag_option_names());
    if (!app_parsed.value) {
        err << "pmzf: " << app_parsed.error << "\n\n" << application_usage();
        return 2;
    }

    if (app_parsed.value->help) {
        out << application_usage();
        return 0;
    }
    if (app_parsed.value->version) {
        out << app::version_text() << '\n';
        out << spagyrist::version_text() << '\n';
        return 0;
    }
    if (app_parsed.value->info) {
        out << spagyrist::runtime_info_text();
        return 0;
    }

    const auto pubmed_parsed = parse_options(args);
    if (!pubmed_parsed.value) {
        err << "pmzf: " << pubmed_parsed.error << "\n\n" << application_usage();
        return 2;
    }

    auto provider = make_provider(*pubmed_parsed.value);
    return app::run(app_parsed.value->query, app_parsed.value->app, *provider, out, err);
}

} // namespace pmzf::pubmed
