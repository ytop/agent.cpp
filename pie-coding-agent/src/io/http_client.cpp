#include "pie/io/http_client.hpp"
#include <curl/curl.h>
#include <cstring>

namespace pie::io {

HttpClient::HttpClient() {
    static bool initialized = [] { curl_global_init(CURL_GLOBAL_DEFAULT); return true; }();
    (void)initialized;
}

HttpClient::~HttpClient() = default;

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

struct SseState {
    SseCallback* callback;
    std::string buffer;
    std::string event_type;
    std::string data;
};

static size_t sse_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* state = static_cast<SseState*>(userdata);
    size_t total = size * nmemb;
    state->buffer.append(ptr, total);

    size_t pos;
    while ((pos = state->buffer.find('\n')) != std::string::npos) {
        std::string line = state->buffer.substr(0, pos);
        state->buffer.erase(0, pos + 1);
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.empty()) {
            if (!state->data.empty()) {
                (*state->callback)(state->event_type, state->data);
                state->event_type.clear();
                state->data.clear();
            }
        } else if (line.starts_with("event:")) {
            state->event_type = line.substr(6);
            while (!state->event_type.empty() && state->event_type[0] == ' ')
                state->event_type.erase(0, 1);
        } else if (line.starts_with("data:")) {
            auto d = line.substr(5);
            if (!d.empty() && d[0] == ' ') d.erase(0, 1);
            if (!state->data.empty()) state->data += "\n";
            state->data += d;
        }
    }
    return total;
}

static curl_slist* build_headers(const std::map<std::string, std::string>& headers) {
    curl_slist* list = nullptr;
    for (const auto& [k, v] : headers) {
        list = curl_slist_append(list, (k + ": " + v).c_str());
    }
    return list;
}

pie::Result<HttpResponse> HttpClient::get(
    const std::string& url,
    const std::map<std::string, std::string>& headers) {

    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("curl_easy_init failed");

    HttpResponse resp{};
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms_);

    auto* hlist = build_headers(headers);
    if (hlist) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hlist);

    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(hlist);

    if (rc != CURLE_OK) {
        curl_easy_cleanup(curl);
        return std::unexpected(std::string("HTTP GET failed: ") + curl_easy_strerror(rc));
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status_code);
    curl_easy_cleanup(curl);
    return resp;
}

pie::Result<HttpResponse> HttpClient::post(
    const std::string& url,
    const std::string& body,
    const std::map<std::string, std::string>& headers) {

    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("curl_easy_init failed");

    HttpResponse resp{};
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms_);

    auto* hlist = build_headers(headers);
    if (hlist) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hlist);

    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(hlist);

    if (rc != CURLE_OK) {
        curl_easy_cleanup(curl);
        return std::unexpected(std::string("HTTP POST failed: ") + curl_easy_strerror(rc));
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status_code);
    curl_easy_cleanup(curl);
    return resp;
}

pie::Result<long> HttpClient::post_sse(
    const std::string& url,
    const std::string& body,
    const std::map<std::string, std::string>& headers,
    SseCallback callback) {

    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected("curl_easy_init failed");

    SseState state{&callback, {}, {}, {}};
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sse_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &state);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms_);

    auto* hlist = build_headers(headers);
    if (hlist) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hlist);

    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(hlist);

    if (rc != CURLE_OK) {
        curl_easy_cleanup(curl);
        return std::unexpected(std::string("SSE POST failed: ") + curl_easy_strerror(rc));
    }
    long status;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    return status;
}

}  // namespace pie::io
