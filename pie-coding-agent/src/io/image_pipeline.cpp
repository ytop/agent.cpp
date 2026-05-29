#include "pie/io/image_pipeline.hpp"
#include <cstdlib>
#include <cstring>

// stb headers are stubs in this project but the API is correct
// Real implementation would #define STB_IMAGE_IMPLEMENTATION here

namespace pie::io {

Result<ImageData> ImagePipeline::decode(const std::vector<uint8_t>& file_data) {
    if (file_data.size() > kMaxFileSize)
        return std::unexpected("image exceeds 20 MB limit");
    if (file_data.empty())
        return std::unexpected("empty image data");

    // Stub: in real impl, use stbi_load_from_memory
    // For now, return error indicating stb not linked
    return std::unexpected("image decode not available (stb stub)");
}

ImageData ImagePipeline::auto_resize(const ImageData& img) {
    if (img.width <= kMaxDimension && img.height <= kMaxDimension) return img;

    double scale = static_cast<double>(kMaxDimension) / std::max(img.width, img.height);
    ImageData resized;
    resized.width = static_cast<int>(img.width * scale);
    resized.height = static_cast<int>(img.height * scale);
    resized.channels = img.channels;
    // Real impl would use stbir_resize_uint8_linear
    resized.pixels.resize(resized.width * resized.height * resized.channels, 0);
    return resized;
}

static const char kBase64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string ImagePipeline::base64_encode(const std::vector<uint8_t>& data) {
    std::string out;
    out.reserve((data.size() + 2) / 3 * 4);
    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < data.size()) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < data.size()) n |= data[i + 2];
        out += kBase64Table[(n >> 18) & 0x3F];
        out += kBase64Table[(n >> 12) & 0x3F];
        out += (i + 1 < data.size()) ? kBase64Table[(n >> 6) & 0x3F] : '=';
        out += (i + 2 < data.size()) ? kBase64Table[n & 0x3F] : '=';
    }
    return out;
}

ImageProtocol ImagePipeline::detect_protocol() {
    if (std::getenv("KITTY_WINDOW_ID")) return ImageProtocol::Kitty;
    // Sixel detection would use DA1/DA2 terminal query
    return ImageProtocol::Placeholder;
}

std::string ImagePipeline::render(const ImageData& img, ImageProtocol proto, int /*width_cells*/) {
    if (proto == ImageProtocol::Kitty) {
        // Kitty graphics protocol: transmit as PNG base64
        auto b64 = base64_encode(img.pixels);
        return "\033_Ga=T,f=32,s=" + std::to_string(img.width) +
               ",v=" + std::to_string(img.height) + ";" + b64 + "\033\\";
    }
    return "[image: " + std::to_string(img.width) + "x" + std::to_string(img.height) + "]";
}

}  // namespace pie::io
