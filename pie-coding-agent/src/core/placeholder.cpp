// pie::core placeholder — ensures libpie has at least one translation unit.
// Replaced in later tasks with real implementations (JsonValue, Logger, etc.).

namespace pie::core {

// Returns the library version string. Used by the SDK public header.
const char* version() noexcept {
    return "0.1.0";
}

}  // namespace pie::core
