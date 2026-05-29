#pragma once

#include "pie/core/result.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace pie::io {

enum class ImageProtocol { Kitty, Sixel, Placeholder };

struct ImageData {
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;
};

class ImagePipeline {
public:
    static constexpr size_t kMaxFileSize = 20 * 1024 * 1024;  // 20 MB
    static constexpr int kMaxDimension = 2000;

    // Decode image from file bytes
    static Result<ImageData> decode(const std::vector<uint8_t>& file_data);

    // Auto-resize: larger dimension → kMaxDimension px
    static ImageData auto_resize(const ImageData& img);

    // Encode to base64
    static std::string base64_encode(const std::vector<uint8_t>& data);

    // Detect best terminal image protocol
    static ImageProtocol detect_protocol();

    // Render image to terminal escape sequence
    static std::string render(const ImageData& img, ImageProtocol proto, int width_cells = 80);
};

}  // namespace pie::io
