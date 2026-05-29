#pragma once

#include "pie/core/result.hpp"
#include <functional>
#include <map>
#include <string>

namespace pie::io {

struct HttpResponse {
    long status_code;
    std::string body;
    std::map<std::string, std::string> headers;
};

using SseCallback = std::function<void(const std::string& event, const std::string& data)>;

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    void set_timeout_ms(long ms) { timeout_ms_ = ms; }

    pie::Result<HttpResponse> get(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {});

    pie::Result<HttpResponse> post(
        const std::string& url,
        const std::string& body,
        const std::map<std::string, std::string>& headers = {});

    // SSE streaming POST — calls callback for each event
    pie::Result<long> post_sse(
        const std::string& url,
        const std::string& body,
        const std::map<std::string, std::string>& headers,
        SseCallback callback);

private:
    long timeout_ms_ = 30000;
};

}  // namespace pie::io
