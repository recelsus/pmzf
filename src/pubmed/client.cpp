#include "pmzf/pubmed/client.hpp"

#include "pmzf/pubmed/http_client.hpp"

#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace pmzf::pubmed {
namespace {

std::string ensure_joined_base(const std::string& base, const std::string& path)
{
    if (base.empty()) {
        return path;
    }
    if (path.empty()) {
        return base;
    }
    if (base.back() == '/' && path.front() == '/') {
        return base.substr(0, base.size() - 1) + path;
    }
    if (base.back() != '/' && path.front() != '/') {
        return base + '/' + path;
    }
    return base + path;
}

std::optional<std::string> get_optional_string(const JsonValue* value)
{
    if (value == nullptr || !value->is_string() || value->as_string().empty()) {
        return std::nullopt;
    }
    return value->as_string();
}

std::optional<std::string> extract_doi(const JsonValue* articleids)
{
    if (articleids == nullptr || !articleids->is_array()) {
        return std::nullopt;
    }
    for (const auto& entry : articleids->as_array()) {
        if (!entry.is_object()) {
            continue;
        }
        const auto* idtype = entry.find("idtype");
        if (idtype == nullptr || !idtype->is_string()) {
            continue;
        }

        std::string lowered = idtype->as_string();
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (lowered == "doi") {
            return get_optional_string(entry.find("value"));
        }
    }
    return std::nullopt;
}

std::vector<std::string> extract_authors(const JsonValue* authors_node)
{
    std::vector<std::string> authors;
    if (authors_node == nullptr || !authors_node->is_array()) {
        return authors;
    }

    for (const auto& author : authors_node->as_array()) {
        if (!author.is_object()) {
            continue;
        }
        const auto* name_node = author.find("name");
        if (name_node != nullptr && name_node->is_string() && !name_node->as_string().empty()) {
            authors.push_back(name_node->as_string());
            if (authors.size() >= 5) {
                break;
            }
        }
    }
    return authors;
}

SummaryItem make_minimal_item(std::string pmid)
{
    SummaryItem item;
    item.pmid = std::move(pmid);
    item.url = "https://pubmed.ncbi.nlm.nih.gov/" + item.pmid + '/';
    return item;
}

} // namespace

EutilsClient::EutilsClient(ClientConfig config)
    : config_(std::move(config))
{
}

std::vector<SummaryItem>
EutilsClient::search(const SearchOptions& options)
{
    const auto ids = esearch_ids(options);
    if (ids.empty()) {
        return {};
    }
    return extract_summary_items(esummary(ids));
}

SummaryItem
EutilsClient::fetch(const std::string& pmid)
{
    const auto items = extract_summary_items(esummary({pmid}));
    if (items.empty()) {
        return make_minimal_item(pmid);
    }
    return items.front();
}

std::vector<std::string>
EutilsClient::esearch_ids(const SearchOptions& options) const
{
    const std::string query = options.date_range ? options.term + " AND " + *options.date_range : options.term;

    std::vector<std::pair<std::string, std::string>> params;
    params.emplace_back("db", "pubmed");
    params.emplace_back("term", query);
    params.emplace_back("retmode", "json");
    params.emplace_back("retmax", std::to_string(options.limit));
    params.emplace_back("retstart", std::to_string(options.offset));
    params.emplace_back("tool", config_.tool_name);
    params.emplace_back("email", config_.email_address);
    if (!config_.api_key.empty()) {
        params.emplace_back("api_key", config_.api_key);
    }

    const auto url = build_url("/esearch.fcgi", params);
    const std::vector<std::pair<std::string, std::string>> headers{
        {"User-Agent", config_.tool_name + '/' + config_.email_address},
        {"Accept", "application/json"},
    };

    const auto body = HttpClient{}.get(url, headers);
    const auto root = parse_json(body);
    const auto* result = root.find("esearchresult");
    if (result == nullptr || !result->is_object()) {
        return {};
    }
    const auto* idlist = result->find("idlist");
    if (idlist == nullptr || !idlist->is_array()) {
        return {};
    }

    std::vector<std::string> ids;
    for (const auto& entry : idlist->as_array()) {
        if (entry.is_string()) {
            ids.push_back(entry.as_string());
        }
    }
    return ids;
}

JsonValue
EutilsClient::esummary(const std::vector<std::string>& pmids) const
{
    if (pmids.empty()) {
        return JsonValue(JsonObject{});
    }

    std::string csv;
    for (std::size_t i = 0; i < pmids.size(); ++i) {
        if (i > 0) {
            csv.push_back(',');
        }
        csv += pmids[i];
    }

    std::vector<std::pair<std::string, std::string>> params;
    params.emplace_back("db", "pubmed");
    params.emplace_back("id", csv);
    params.emplace_back("retmode", "json");
    params.emplace_back("tool", config_.tool_name);
    params.emplace_back("email", config_.email_address);
    if (!config_.api_key.empty()) {
        params.emplace_back("api_key", config_.api_key);
    }

    const auto url = build_url("/esummary.fcgi", params);
    const std::vector<std::pair<std::string, std::string>> headers{
        {"User-Agent", config_.tool_name + '/' + config_.email_address},
        {"Accept", "application/json"},
    };

    return parse_json(HttpClient{}.get(url, headers));
}

std::string
EutilsClient::build_url(
    const std::string& path,
    const std::vector<std::pair<std::string, std::string>>& params) const
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        throw std::runtime_error("curl initialization failed");
    }

    std::ostringstream query;
    bool first = true;
    for (const auto& [key, value] : params) {
        char* encoded = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.size()));
        if (encoded == nullptr) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("failed to encode query parameter");
        }
        if (!first) {
            query << '&';
        }
        first = false;
        query << key << '=' << encoded;
        curl_free(encoded);
    }

    curl_easy_cleanup(curl);

    std::string url = ensure_joined_base(config_.base_url, path);
    if (!params.empty()) {
        url.push_back('?');
        url += query.str();
    }
    return url;
}

std::vector<SummaryItem>
extract_summary_items(const JsonValue& esummary_json)
{
    const auto* result_node = esummary_json.find("result");
    if (result_node == nullptr || !result_node->is_object()) {
        return {};
    }
    const auto* uids_node = result_node->find("uids");
    if (uids_node == nullptr || !uids_node->is_array()) {
        return {};
    }

    std::vector<SummaryItem> items;
    for (const auto& uid_value : uids_node->as_array()) {
        if (!uid_value.is_string()) {
            continue;
        }

        SummaryItem item = make_minimal_item(uid_value.as_string());
        const auto* record_node = result_node->find(item.pmid);
        if (record_node != nullptr && record_node->is_object()) {
            if (const auto* pmid_node = record_node->find("uid");
                pmid_node != nullptr && pmid_node->is_string() && !pmid_node->as_string().empty()) {
                item = make_minimal_item(pmid_node->as_string());
            }
            item.title = get_optional_string(record_node->find("title"));
            item.journal = get_optional_string(record_node->find("fulljournalname"));
            item.pubdate = get_optional_string(record_node->find("pubdate"));
            item.doi = extract_doi(record_node->find("articleids"));
            item.authors = extract_authors(record_node->find("authors"));
        }
        items.push_back(std::move(item));
    }
    return items;
}

} // namespace pmzf::pubmed
