#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include "pie/core/logger.hpp"
#include "pie/auth/auth_storage.hpp"
#include "pie/models/model_registry.hpp"
#include "pie/settings/settings_manager.hpp"
#include "pie/session/session_manager.hpp"
#include "pie/resources/resource_loader.hpp"
#include "pie/tools/tool_host.hpp"
#include "pie/tools/define_tool.hpp"
#include "pie/agent/agent.hpp"
#include "pie/agent/events.hpp"
#include "pie/agent/agent_session.hpp"
#include "pie/agent/agent_session_runtime.hpp"
#include "pie/version.hpp"

namespace pie {

// Re-exports of core classes into pie namespace for convenient public usage
using AuthStorage = auth::AuthStorage;
using ModelRegistry = models::ModelRegistry;
using SettingsManager = settings::SettingsManager;
using SessionManager = session::SessionManager;
using ResourceLoader = resources::ResourceLoader;
using Tool = tools::Tool;
using ToolHost = tools::ToolHost;

} // namespace pie
