#include "pmzf/pubmed/options.hpp"

#include <cstdlib>

namespace pmzf::pubmed {
namespace {

std::string read_env_or_default(const char* name, const char* fallback)
{
    if (const char* value = std::getenv(name); value != nullptr && *value != '\0') {
        return std::string(value);
    }
    return std::string(fallback);
}

} // namespace

ClientConfig load_client_config()
{
    ClientConfig config;
    config.base_url = read_env_or_default("PMZF_BASE_URL", config.base_url.c_str());
    config.tool_name = read_env_or_default("PMZF_TOOL_NAME", config.tool_name.c_str());
    config.email_address = read_env_or_default("PMZF_EMAIL_ADDR", config.email_address.c_str());
    config.api_key = read_env_or_default("NCBI_API_KEY", "");
    return config;
}

} // namespace pmzf::pubmed
