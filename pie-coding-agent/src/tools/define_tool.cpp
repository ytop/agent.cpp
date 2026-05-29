#include "pie/tools/define_tool.hpp"
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <stdexcept>

namespace pie {

class CustomTool : public tools::Tool {
public:
    CustomTool(std::string name, std::string label, std::string desc, core::JsonValue schema,
               std::function<Result<core::JsonValue>(const core::JsonValue&)> execute_fn)
        : name_(std::move(name)), label_(std::move(label)), desc_(std::move(desc)),
          schema_(std::move(schema)), execute_fn_(std::move(execute_fn)) {}

    std::string name() const override { return name_; }
    std::string label() const override { return label_; }
    std::string description() const override { return desc_; }
    core::JsonValue parameters_schema() const override { return schema_; }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        return execute_fn_(params);
    }

private:
    std::string name_;
    std::string label_;
    std::string desc_;
    core::JsonValue schema_;
    std::function<Result<core::JsonValue>(const core::JsonValue&)> execute_fn_;
};

Result<std::shared_ptr<tools::Tool>> define_tool(ToolDefinition def) {
    if (def.name.empty() || def.label.empty() || def.description.empty()) {
        return std::unexpected("Missing required fields in tool definition");
    }

    // Validate parameters_schema via valijson
    try {
        nlohmann::json schema_plain = nlohmann::json::parse(def.parameters_schema.dump());
        valijson::Schema schema;
        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter adapter(schema_plain);
        parser.populateSchema(adapter, schema);
    } catch (const std::exception& e) {
        return std::unexpected(std::string("Invalid JSON schema: ") + e.what());
    } catch (...) {
        return std::unexpected("Invalid JSON schema: unknown error");
    }

    return std::make_shared<CustomTool>(
        std::move(def.name),
        std::move(def.label),
        std::move(def.description),
        std::move(def.parameters_schema),
        std::move(def.execute)
    );
}

} // namespace pie
