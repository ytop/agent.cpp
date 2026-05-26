/*
 * stb_image_resize2.h - v2.10 - public domain image resizing
 *
 * PLACEHOLDER STUB — replace with the real file from:
 *   https://github.com/nothings/stb/blob/5c20573/stb_image_resize2.h
 *
 * Vendored from: nothings/stb commit 5c20573
 *
 * This stub declares the subset of the stb_image_resize2 public API used
 * by pie-coding-agent (ImagePipeline auto-resize). It is sufficient for
 * compilation but does NOT contain the actual implementation. Replace this
 * file with the real stb_image_resize2.h to get working image resizing.
 */

#ifndef STBIR_INCLUDE_STB_IMAGE_RESIZE2_H
#define STBIR_INCLUDE_STB_IMAGE_RESIZE2_H

#include <cstddef>

// Allow overriding linkage
#ifndef STBIRDEF
#ifdef STB_IMAGE_RESIZE_STATIC
#define STBIRDEF static
#else
#define STBIRDEF extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//
// TYPES
//

// Pixel layout enum — describes how channels are arranged
typedef enum {
    STBIR_1CHANNEL = 1,
    STBIR_2CHANNEL = 2,
    STBIR_RGB      = 3,
    STBIR_RGBA     = 4,
    STBIR_4CHANNEL = 4,
    STBIR_RGBA_PM  = 5,   // premultiplied alpha
    STBIR_BGRA     = 6,
    STBIR_BGRA_PM  = 7,
} stbir_pixel_layout;

// Datatype of pixel components
typedef enum {
    STBIR_TYPE_UINT8            = 0,
    STBIR_TYPE_UINT8_SRGB       = 1,
    STBIR_TYPE_UINT8_SRGB_ALPHA = 2,
    STBIR_TYPE_UINT16           = 3,
    STBIR_TYPE_FLOAT            = 4,
    STBIR_TYPE_HALF_FLOAT       = 5,
} stbir_datatype;

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API
//

// Simple linear resize for 8-bit images.
// Returns a pointer to the output pixels (same as output_pixels) on success,
// or NULL on failure.
//
// Parameters:
//   input_pixels   - source image data
//   input_w        - source width in pixels
//   input_h        - source height in pixels
//   input_stride   - source row stride in bytes (0 = tightly packed)
//   output_pixels  - destination buffer (must be pre-allocated)
//   output_w       - destination width in pixels
//   output_h       - destination height in pixels
//   output_stride  - destination row stride in bytes (0 = tightly packed)
//   pixel_layout   - channel layout (e.g. STBIR_RGBA)
//
STBIRDEF unsigned char *stbir_resize_uint8_linear(
    const unsigned char *input_pixels,
    int input_w,
    int input_h,
    int input_stride,
    unsigned char *output_pixels,
    int output_w,
    int output_h,
    int output_stride,
    stbir_pixel_layout pixel_layout
);

#ifdef __cplusplus
}
#endif

#endif // STBIR_INCLUDE_STB_IMAGE_RESIZE2_H

//////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//

#ifdef STB_IMAGE_RESIZE_IMPLEMENTATION
#ifndef STBIR_IMPLEMENTATION_DEFINED
#define STBIR_IMPLEMENTATION_DEFINED

// Stub implementation — provides a linkable symbol but no real resizing.
// Replace this entire file with the real stb_image_resize2.h for actual
// functionality.

STBIRDEF unsigned char *stbir_resize_uint8_linear(
    const unsigned char * /*input_pixels*/,
    int /*input_w*/,
    int /*input_h*/,
    int /*input_stride*/,
    unsigned char *output_pixels,
    int output_w,
    int output_h,
    int output_stride,
    stbir_pixel_layout pixel_layout)
{
    // Stub: zero-fill the output buffer as a no-op placeholder.
    if (!output_pixels) return (unsigned char *)0;

    int channels = (int)pixel_layout;
    if (channels < 1) channels = 1;
    if (channels > 4) channels = 4;

    int stride = output_stride ? output_stride : output_w * channels;
    for (int y = 0; y < output_h; ++y) {
        unsigned char *row = output_pixels + y * stride;
        for (int x = 0; x < output_w * channels; ++x) {
            row[x] = 0;
        }
    }
    return output_pixels;
}

#endif // STBIR_IMPLEMENTATION_DEFINED
#endif // STB_IMAGE_RESIZE_IMPLEMENTATION
