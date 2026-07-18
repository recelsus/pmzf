#include "pmzf/app/application.hpp"

#include "spagyrist/spagyrist.hpp"

#include <exception>
#include <ostream>

namespace pmzf::app {

int run(
    const std::string& query,
    const Options& options,
    provider::Provider& provider,
    std::ostream& out,
    std::ostream& err)
{
    try {
        auto candidates = provider.search(query);

        const auto candidate_validation = spagyrist::validate(spagyrist::candidate_list{candidates});
        if (!candidate_validation.ok()) {
            err << "invalid candidate data\n";
            return 1;
        }

        const auto selected = spagyrist::select_candidate(options.spagyrist.selector, candidates);
        if (!selected.selected) {
            if (selected.status == spagyrist::selector_status::no_selection && !selected.message.empty()) {
                err << selected.message << '\n';
            }
            return 0;
        }

        auto document = provider.fetch(*selected.selected);

        const auto document_validation = spagyrist::validate(document);
        if (!document_validation.ok()) {
            err << "invalid document data\n";
            return 1;
        }

        const auto rendered = spagyrist::render(document, options.spagyrist.output_format);
        spagyrist::write_output(options.spagyrist.output, rendered, out);
        return 0;
    } catch (const std::exception& error) {
        err << "pmzf: " << error.what() << '\n';
        return 1;
    }
}

} // namespace pmzf::app
