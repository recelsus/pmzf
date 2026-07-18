#include "pmzf/app/version.hpp"

namespace pmzf::app {

std::string version()
{
    return PMZF_VERSION;
}

std::string version_text()
{
    return "pmzf " + version();
}

} // namespace pmzf::app
