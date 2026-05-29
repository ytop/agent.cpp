#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace pie::sdk::export_html {

class Exporter {
public:
    // Render session messages to self-contained HTML
    static std::string render(const std::vector<core::JsonValue>& messages);

    // Write HTML to file
    static Result<void> export_to(
        const std::vector<core::JsonValue>& messages,
        const std::filesystem::path& output_path);
};

class GistUploader {
public:
    // Upload HTML content to GitHub Gist, returns gist URL
    static Result<std::string> upload(
        const std::string& html_content,
        const std::string& github_token,
        const std::string& viewer_url = "");
};

}  // namespace pie::sdk::export_html
