#include "pie/sdk/exporter.hpp"
#include "pie/io/http_client.hpp"
#include <fstream>

namespace pie::sdk::export_html {

std::string Exporter::render(const std::vector<core::JsonValue>& messages) {
    std::string html = R"(<!DOCTYPE html><html><head><meta charset="utf-8">
<title>Pie Session Export</title><style>
body{font-family:system-ui,sans-serif;max-width:800px;margin:0 auto;padding:20px;background:#1e1e2e;color:#cdd6f4}
.msg{margin:12px 0;padding:8px 12px;border-radius:6px}
.user{background:#313244}.assistant{background:#1e1e2e;border:1px solid #45475a}
</style></head><body>)";

    for (const auto& msg : messages) {
        auto role = msg.value("role", "system");
        auto content = msg.value("content", "");
        html += "<div class=\"msg " + role + "\"><strong>" + role + ":</strong> " + content + "</div>\n";
    }

    html += "</body></html>";
    return html;
}

Result<void> Exporter::export_to(
    const std::vector<core::JsonValue>& messages,
    const std::filesystem::path& output_path) {
    auto html = render(messages);
    std::ofstream f(output_path);
    if (!f) return std::unexpected("cannot write: " + output_path.string());
    f << html;
    return {};
}

Result<std::string> GistUploader::upload(
    const std::string& html_content,
    const std::string& github_token,
    const std::string& viewer_url) {

    io::HttpClient http;
    http.set_timeout_ms(30000);

    core::JsonValue body = {
        {"description", "Pie session export"},
        {"public", false},
        {"files", {{"session.html", {{"content", html_content}}}}}
    };

    auto resp = http.post("https://api.github.com/gists", body.dump(),
        {{"Authorization", "Bearer " + github_token},
         {"Content-Type", "application/json"},
         {"User-Agent", "pie-coding-agent"}});

    if (!resp) return std::unexpected(resp.error());
    if (resp->status_code != 201)
        return std::unexpected("Gist upload failed: HTTP " + std::to_string(resp->status_code));

    auto result = core::JsonValue::parse(resp->body);
    auto gist_id = result.value("id", "");
    auto url = viewer_url.empty()
        ? result.value("html_url", "")
        : viewer_url + "/" + gist_id;
    return url;
}

}  // namespace pie::sdk::export_html
