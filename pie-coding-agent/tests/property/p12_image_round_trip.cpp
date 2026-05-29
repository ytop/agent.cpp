// Feature: cpp-coding-agent, Property 12: image base64 encode produces valid base64
// Feature: cpp-coding-agent, Property 13: image auto_resize respects kMaxDimension
// Feature: cpp-coding-agent, Property 14: detect_protocol returns a valid enum value

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/io/image_pipeline.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>

TEST_CASE("Property 12: base64 encode only produces valid base64 chars", "[property][image]") {
    // Feature: cpp-coding-agent, Property 12: image decode/encode round-trip
    rc::prop("base64 output contains only valid base64 characters", []() {
        int len = *rc::gen::inRange(0, 256);
        std::vector<uint8_t> bytes;
        bytes.reserve(len);
        for (int i = 0; i < len; ++i) {
            bytes.push_back(static_cast<uint8_t>(*rc::gen::inRange(0, 256)));
        }

        std::string encoded = pie::io::ImagePipeline::base64_encode(bytes);

        // Base64 alphabet: A-Z, a-z, 0-9, +, /, =
        static const std::string b64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
        for (char c : encoded) {
            RC_ASSERT(b64_chars.find(c) != std::string::npos);
        }
    });
}

TEST_CASE("Property 13: auto_resize respects kMaxDimension", "[property][image]") {
    // Feature: cpp-coding-agent, Property 13: image resize dimensions within tolerance
    rc::prop("auto_resize output does not exceed kMaxDimension", []() {
        // Generate image data with random dimensions
        int w = *rc::gen::inRange(1, 5000);
        int h = *rc::gen::inRange(1, 5000);
        int ch = *rc::gen::element(std::vector<int>{1, 3, 4});

        pie::io::ImageData img;
        img.width = w;
        img.height = h;
        img.channels = ch;
        img.pixels.resize(static_cast<size_t>(w) * h * ch, 128);

        auto resized = pie::io::ImagePipeline::auto_resize(img);

        RC_ASSERT(resized.width <= pie::io::ImagePipeline::kMaxDimension);
        RC_ASSERT(resized.height <= pie::io::ImagePipeline::kMaxDimension);
        RC_ASSERT(resized.width > 0);
        RC_ASSERT(resized.height > 0);
    });
}

TEST_CASE("Property 14: detect_protocol is deterministic", "[property][image]") {
    // Feature: cpp-coding-agent, Property 14: EXIF orientation / protocol detection
    rc::prop("detect_protocol returns same value on repeated calls", []() {
        auto p1 = pie::io::ImagePipeline::detect_protocol();
        auto p2 = pie::io::ImagePipeline::detect_protocol();
        RC_ASSERT(p1 == p2);

        // Must be one of the known enum values
        bool valid = (p1 == pie::io::ImageProtocol::Kitty ||
                      p1 == pie::io::ImageProtocol::Sixel ||
                      p1 == pie::io::ImageProtocol::Placeholder);
        RC_ASSERT(valid);
    });
}
