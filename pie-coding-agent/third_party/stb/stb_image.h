/*
 * stb_image.h - v2.29 - public domain image loader
 *
 * PLACEHOLDER STUB — replace with the real file from:
 *   https://github.com/nothings/stb/blob/5c20573/stb_image.h
 *
 * Vendored from: nothings/stb commit 5c20573
 *
 * This stub declares the subset of the stb_image public API used by
 * pie-coding-agent (ImagePipeline). It is sufficient for compilation
 * but does NOT contain the actual implementation. Replace this file
 * with the real stb_image.h to get working image decoding.
 *
 * Supported formats (real library): JPEG, PNG, BMP, PSD, TGA, GIF,
 * HDR, PIC, PNM
 */

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#include <cstdlib>

// Allow overriding linkage
#ifndef STBIDEF
#ifdef STB_IMAGE_STATIC
#define STBIDEF static
#else
#define STBIDEF extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on files, memory, callbacks
//

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

// Load an image from a file path.
// Returns pixel data in the requested number of channels (0 = auto-detect).
// On success, *x, *y, *channels_in_file are filled in.
// The returned pointer must be freed with stbi_image_free().
STBIDEF stbi_uc *stbi_load(
    const char *filename,
    int *x,
    int *y,
    int *channels_in_file,
    int desired_channels
);

// Load an image from a memory buffer.
STBIDEF stbi_uc *stbi_load_from_memory(
    const stbi_uc *buffer,
    int len,
    int *x,
    int *y,
    int *channels_in_file,
    int desired_channels
);

// Get image dimensions and channel count from a memory buffer without
// decoding the full image.
// Returns 1 on success, 0 on failure.
STBIDEF int stbi_info_from_memory(
    const stbi_uc *buffer,
    int len,
    int *x,
    int *y,
    int *comp
);

// Free pixel data returned by stbi_load* functions.
STBIDEF void stbi_image_free(void *retval_from_stbi_load);

// Get a human-readable reason for the most recent failure.
STBIDEF const char *stbi_failure_reason(void);

//////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//

#ifdef STB_IMAGE_IMPLEMENTATION

// Stub implementation — provides linkable symbols but no real decoding.
// Replace this entire file with the real stb_image.h for actual functionality.

static const char *stbi__g_failure_reason = "stub: not implemented";

STBIDEF stbi_uc *stbi_load(
    const char * /*filename*/,
    int *x,
    int *y,
    int *channels_in_file,
    int /*desired_channels*/)
{
    if (x) *x = 0;
    if (y) *y = 0;
    if (channels_in_file) *channels_in_file = 0;
    stbi__g_failure_reason = "stub: stbi_load not implemented";
    return nullptr;
}

STBIDEF stbi_uc *stbi_load_from_memory(
    const stbi_uc * /*buffer*/,
    int /*len*/,
    int *x,
    int *y,
    int *channels_in_file,
    int /*desired_channels*/)
{
    if (x) *x = 0;
    if (y) *y = 0;
    if (channels_in_file) *channels_in_file = 0;
    stbi__g_failure_reason = "stub: stbi_load_from_memory not implemented";
    return nullptr;
}

STBIDEF int stbi_info_from_memory(
    const stbi_uc * /*buffer*/,
    int /*len*/,
    int *x,
    int *y,
    int *comp)
{
    if (x) *x = 0;
    if (y) *y = 0;
    if (comp) *comp = 0;
    stbi__g_failure_reason = "stub: stbi_info_from_memory not implemented";
    return 0;
}

STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
    free(retval_from_stbi_load);
}

STBIDEF const char *stbi_failure_reason(void)
{
    return stbi__g_failure_reason;
}

#endif // STB_IMAGE_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // STBI_INCLUDE_STB_IMAGE_H
