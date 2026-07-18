#include "pmzf/pubmed/provider.hpp"

#include "pmzf/pubmed/mapper.hpp"

#include <utility>

namespace pmzf::pubmed {

Provider::Provider(std::unique_ptr<Client> client, Options options)
    : client_(std::move(client))
    , options_(std::move(options))
{
}

std::vector<spagyrist::candidate>
Provider::search(const std::string& query)
{
    SearchOptions search_options;
    search_options.term = query;
    search_options.date_range = options_.date_range;
    search_options.limit = options_.limit;

    auto items = client_->search(search_options);
    item_cache_.clear();
    for (const auto& item : items) {
        item_cache_.emplace(item.pmid, item);
    }
    return to_candidates(items);
}

spagyrist::document
Provider::fetch(const spagyrist::selection& selected)
{
    auto item = client_->fetch(selected.item.id);
    if (!item.title) {
        if (const auto found = item_cache_.find(selected.item.id); found != item_cache_.end()) {
            auto cached = found->second;
            cached.abstract_text = item.abstract_text;
            item = std::move(cached);
        }
    }
    return to_document(item);
}

std::unique_ptr<provider::Provider>
make_provider(const Options& options)
{
    std::unique_ptr<Client> client;
    switch (options.client) {
    case ClientKind::http:
        client = std::make_unique<EutilsClient>(options.config);
        break;
    }

    return std::make_unique<Provider>(std::move(client), options);
}

} // namespace pmzf::pubmed
