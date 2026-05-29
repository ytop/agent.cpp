#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <string>
#include <vector>

namespace pie::wire {

class JsonSchemaValidator {
public:
    // Validate a JsonValue against a JSON Schema object.
    // Returns empty vector on success, or list of validation error messages.
    static std::vector<std::string> validate(
        const core::JsonValue& schema,
        const core::JsonValue& document);
};

}  // namespace pie::wire
