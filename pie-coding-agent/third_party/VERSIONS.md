# Third-Party Vendored Libraries

This file records the exact commit hashes of all vendored header-only
libraries in `pie-coding-agent/third_party/`.

| Library | Source | Commit | Path |
|---------|--------|--------|------|
| stb_image.h | [nothings/stb](https://github.com/nothings/stb) | `5c20573` | `stb/stb_image.h` |
| stb_image_resize2.h | [nothings/stb](https://github.com/nothings/stb) | `5c20573` | `stb/stb_image_resize2.h` |
| dtl.hpp | [cubicdaiya/dtl](https://github.com/cubicdaiya/dtl) | `f3a1b22` | `dtl/dtl.hpp` |

## Notes

- These are **placeholder stubs** that declare the public API surface used
  by pie-coding-agent. They compile and link but do not provide real
  functionality. Replace each file with the actual vendored source from the
  commit listed above for production use.
- stb libraries use the single-header pattern: define `STB_IMAGE_IMPLEMENTATION`
  or `STB_IMAGE_RESIZE_IMPLEMENTATION` in exactly one `.cpp` file before
  including the header to get the implementation.
- dtl is a header-only C++ template library; no implementation macro needed.
