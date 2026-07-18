#pragma once

#include "pmzf/pubmed/options.hpp"
#include "pmzf/support/json.hpp"

#include <optional>
#include <string>
#include <vector>

namespace pmzf::pubmed {

struct SearchOptions {
    std::string term;
    std::optional<std::string> date_range;
    std::size_t limit{20};
    std::size_t offset{0};
};

struct SummaryItem {
    std::string pmid;
    std::optional<std::string> title;
    std::optional<std::string> journal;
    std::optional<std::string> pubdate;
    std::optional<std::string> doi;
    std::vector<std::string> authors;
    std::string url;
};

class Client {
public:
    virtual ~Client() = default;

    virtual std::vector<SummaryItem>
    search(const SearchOptions& options) = 0;

    virtual SummaryItem
    fetch(const std::string& pmid) = 0;
};

class EutilsClient final : public Client {
public:
    explicit EutilsClient(ClientConfig config);

    std::vector<SummaryItem>
    search(const SearchOptions& options) override;

    SummaryItem
    fetch(const std::string& pmid) override;

private:
    [[nodiscard]] std::vector<std::string>
    esearch_ids(const SearchOptions& options) const;

    [[nodiscard]] JsonValue
    esummary(const std::vector<std::string>& pmids) const;

    [[nodiscard]] std::string
    build_url(
        const std::string& path,
        const std::vector<std::pair<std::string, std::string>>& params) const;

    ClientConfig config_;
};

[[nodiscard]] std::vector<SummaryItem>
extract_summary_items(const JsonValue& esummary_json);

} // namespace pmzf::pubmed
