#pragma once

#include "pmzf/pubmed/client.hpp"
#include "pmzf/pubmed/options.hpp"
#include "pmzf/provider/provider.hpp"

#include <memory>
#include <unordered_map>

namespace pmzf::pubmed {

class Provider final : public provider::Provider {
public:
    Provider(std::unique_ptr<Client> client, Options options);

    std::vector<spagyrist::candidate>
    search(const std::string& query) override;

    spagyrist::document
    fetch(const spagyrist::selection& selected) override;

private:
    std::unique_ptr<Client> client_;
    Options options_;
    std::unordered_map<std::string, SummaryItem> item_cache_;
};

[[nodiscard]] std::unique_ptr<provider::Provider>
make_provider(const Options& options);

} // namespace pmzf::pubmed
