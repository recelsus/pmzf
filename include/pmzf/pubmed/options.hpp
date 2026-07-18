#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace pmzf::pubmed {

enum class ClientKind {
    http,
};

struct ClientConfig {
    std::string base_url{"https://eutils.ncbi.nlm.nih.gov/entrez/eutils"};
    std::string tool_name{"pmzf"};
    std::string email_address{"me@example.com"};
    std::string api_key;
};

struct Options {
    ClientKind client{ClientKind::http};
    ClientConfig config;
    std::optional<std::string> date_range;
    std::size_t limit{20};
};

[[nodiscard]] ClientConfig load_client_config();

} // namespace pmzf::pubmed
