#include "pie/modes/modes.hpp"
#include "pie/session/session_file.hpp"
#include "pie/sdk/exporter.hpp"
#include <iostream>
#include <fstream>

namespace pie::modes {

int ExportMode::run(const std::filesystem::path& in_path, const std::optional<std::filesystem::path>& out_path) {
    auto file_res = session::SessionFile::open(in_path);
    if (!file_res) {
        std::cerr << "error: failed to open session file: " << file_res.error() << "\n";
        return 1;
    }

    // Filter and collect all entries that represent conversation messages (contain role & content)
    std::vector<core::JsonValue> messages;
    for (const auto& entry : file_res->entries()) {
        if (entry.value.contains("role") && entry.value.contains("content")) {
            messages.push_back(entry.value);
        }
    }

    std::string html = sdk::export_html::Exporter::render(messages);

    if (out_path) {
        std::ofstream out(*out_path);
        if (!out) {
            std::cerr << "error: failed to open output file: " << out_path->string() << "\n";
            return 1;
        }
        out << html;
    } else {
        std::cout << html << "\n";
    }

    return 0;
}

} // namespace pie::modes
