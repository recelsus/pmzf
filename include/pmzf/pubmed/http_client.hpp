#pragma once

#include <string>
#include <utility>
#include <vector>

namespace pmzf::pubmed {

class HttpClient {
public:
    [[nodiscard]] std::string
    get(
        const std::string& url,
        const std::vector<std::pair<std::string, std::string>>& headers = {}) const;
};

} // namespace pmzf::pubmed
