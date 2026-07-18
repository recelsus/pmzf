#include "pmzf/app/version.hpp"

#include "test_support.hpp"

namespace {

void version_reports_project_version()
{
    PMZF_CHECK(pmzf::app::version() == "0.1.0");
    PMZF_CHECK(pmzf::app::version_text() == "pmzf 0.1.0");
}

} // namespace

void run_version_tests()
{
    version_reports_project_version();
}
