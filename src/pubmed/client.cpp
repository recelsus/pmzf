#include "pmzf/pubmed/client.hpp"

#include "pmzf/pubmed/http_client.hpp"

#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>
#include <string_view>
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

std::string trim(std::string value)
{
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();

    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

std::string decode_xml_entities(std::string_view text)
{
    std::string output;
    output.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '&') {
            output.push_back(text[i]);
            continue;
        }

        const auto semicolon = text.find(';', i + 1);
        if (semicolon == std::string_view::npos) {
            output.push_back(text[i]);
            continue;
        }

        const auto entity = text.substr(i, semicolon - i + 1);
        if (entity == "&amp;") {
            output.push_back('&');
        } else if (entity == "&lt;") {
            output.push_back('<');
        } else if (entity == "&gt;") {
            output.push_back('>');
        } else if (entity == "&quot;") {
            output.push_back('"');
        } else if (entity == "&apos;") {
            output.push_back('\'');
        } else {
            output.append(entity);
        }
        i = semicolon;
    }

    return output;
}

std::string strip_xml_tags(std::string_view text)
{
    std::string output;
    output.reserve(text.size());
    bool inside_tag = false;

    for (const char ch : text) {
        if (ch == '<') {
            inside_tag = true;
            continue;
        }
        if (ch == '>') {
            inside_tag = false;
            continue;
        }
        if (!inside_tag) {
            output.push_back(ch);
        }
    }

    return trim(decode_xml_entities(output));
}

std::optional<std::string> extract_attribute(std::string_view tag, std::string_view name)
{
    const auto name_pos = tag.find(name);
    if (name_pos == std::string_view::npos) {
        return std::nullopt;
    }

    auto pos = name_pos + name.size();
    while (pos < tag.size() && std::isspace(static_cast<unsigned char>(tag[pos])) != 0) {
        ++pos;
    }
    if (pos >= tag.size() || tag[pos] != '=') {
        return std::nullopt;
    }
    ++pos;
    while (pos < tag.size() && std::isspace(static_cast<unsigned char>(tag[pos])) != 0) {
        ++pos;
    }
    if (pos >= tag.size() || (tag[pos] != '"' && tag[pos] != '\'')) {
        return std::nullopt;
    }

    const char quote = tag[pos++];
    const auto end = tag.find(quote, pos);
    if (end == std::string_view::npos) {
        return std::nullopt;
    }
    return decode_xml_entities(tag.substr(pos, end - pos));
}

std::optional<std::string> extract_abstract_text(std::string_view xml)
{
    std::vector<std::string> parts;
    std::size_t pos = 0;

    while (true) {
        const auto open = xml.find("<AbstractText", pos);
        if (open == std::string_view::npos) {
            break;
        }
        const auto open_end = xml.find('>', open);
        if (open_end == std::string_view::npos) {
            break;
        }
        const auto close = xml.find("</AbstractText>", open_end + 1);
        if (close == std::string_view::npos) {
            break;
        }

        const auto open_tag = xml.substr(open, open_end - open + 1);
        auto text = strip_xml_tags(xml.substr(open_end + 1, close - open_end - 1));
        if (!text.empty()) {
            if (const auto label = extract_attribute(open_tag, "Label"); label && !label->empty()) {
                text = *label + ": " + text;
            }
            parts.push_back(std::move(text));
        }
        pos = close + std::string_view{"</AbstractText>"}.size();
    }

    if (parts.empty()) {
        return std::nullopt;
    }

    std::ostringstream output;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            output << "\n\n";
        }
        output << parts[i];
    }
    return output.str();
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
    auto item = items.empty() ? make_minimal_item(pmid) : items.front();
    if (const auto abstract = efetch_abstract(item.pmid)) {
        item.abstract_text = abstract;
    }
    return item;
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

std::optional<std::string>
EutilsClient::efetch_abstract(const std::string& pmid) const
{
    std::vector<std::pair<std::string, std::string>> params;
    params.emplace_back("db", "pubmed");
    params.emplace_back("id", pmid);
    params.emplace_back("retmode", "xml");
    params.emplace_back("tool", config_.tool_name);
    params.emplace_back("email", config_.email_address);
    if (!config_.api_key.empty()) {
        params.emplace_back("api_key", config_.api_key);
    }

    const auto url = build_url("/efetch.fcgi", params);
    const std::vector<std::pair<std::string, std::string>> headers{
        {"User-Agent", config_.tool_name + '/' + config_.email_address},
        {"Accept", "application/xml"},
    };

    return extract_abstract_text(HttpClient{}.get(url, headers));
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
