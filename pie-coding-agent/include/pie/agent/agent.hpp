#pragma once

#include "pie/auth/auth_storage.hpp"
#include "pie/models/model_registry.hpp"
#include "pie/settings/settings_manager.hpp"
#include "pie/resources/resource_loader.hpp"
#include "pie/tools/tool_host.hpp"
#include <memory>

namespace pie {

class Agent {
public:
    Agent() = default;
    ~Agent() = default;

    std::shared_ptr<settings::SettingsManager> settings_manager;
    std::shared_ptr<auth::AuthStorage> auth_storage;
    std::shared_ptr<models::ModelRegistry> model_registry;
    std::shared_ptr<resources::ResourceLoader> resource_loader;
    std::shared_ptr<tools::ToolHost> tool_host;
};

} // namespace pie
