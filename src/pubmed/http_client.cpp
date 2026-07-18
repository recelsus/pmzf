#include "pmzf/pubmed/http_client.hpp"

#include <curl/curl.h>

#include <stdexcept>

namespace pmzf::pubmed {
namespace {

std::size_t write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
{
    const auto total = size * nmemb;
    auto* buffer = static_cast<std::string*>(userdata);
    buffer->append(ptr, total);
    return total;
}

} // namespace

std::string HttpClient::get(
    const std::string& url,
    const std::vector<std::pair<std::string, std::string>>& headers) const
{
    std::string body;

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        throw std::runtime_error("curl initialization failed");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    curl_slist* header_list = nullptr;
    for (const auto& header : headers) {
        const std::string composed = header.first + ": " + header.second;
        header_list = curl_slist_append(header_list, composed.c_str());
    }
    if (header_list != nullptr) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }

    char error_buffer[CURL_ERROR_SIZE] = {0};
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);

    const CURLcode result = curl_easy_perform(curl);
    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

    if (header_list != nullptr) {
        curl_slist_free_all(header_list);
    }
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        const std::string message = *error_buffer != '\0' ? std::string(error_buffer) : curl_easy_strerror(result);
        throw std::runtime_error("curl request failed: " + message);
    }
    if (status_code < 200 || status_code >= 300) {
        throw std::runtime_error("unexpected HTTP status: " + std::to_string(status_code));
    }

    return body;
}

} // namespace pmzf::pubmed
