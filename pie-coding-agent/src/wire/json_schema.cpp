#include "pie/wire/json_schema.hpp"

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>
#include <valijson/validation_results.hpp>

namespace pie::wire {

std::vector<std::string> JsonSchemaValidator::validate(
    const core::JsonValue& schema_json,
    const core::JsonValue& document) {

    std::vector<std::string> errors;

    // Convert ordered_json to json for valijson compatibility
    nlohmann::json schema_plain = nlohmann::json::parse(schema_json.dump());
    nlohmann::json doc_plain = nlohmann::json::parse(document.dump());

    valijson::Schema schema;
    valijson::SchemaParser parser;
    valijson::adapters::NlohmannJsonAdapter schema_adapter(schema_plain);

    try {
        parser.populateSchema(schema_adapter, schema);
    } catch (const std::exception& e) {
        errors.push_back(std::string("schema parse error: ") + e.what());
        return errors;
    }

    valijson::Validator validator;
    valijson::ValidationResults results;
    valijson::adapters::NlohmannJsonAdapter doc_adapter(doc_plain);

    if (!validator.validate(schema, doc_adapter, &results)) {
        valijson::ValidationResults::Error error;
        while (results.popError(error)) {
            std::string path;
            for (const auto& p : error.context) path += p;
            errors.push_back(path + ": " + error.description);
        }
    }

    return errors;
}

}  // namespace pie::wire
