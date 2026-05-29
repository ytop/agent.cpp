#pragma once

#include "pie/core/result.hpp"
#include "pie/session/session_file.hpp"

namespace pie::session {

// Migrate v1 session to v3: assign IDs and parentIds
Result<void> migrate_v1_to_v3(SessionFile& file);

// Migrate v2 session to v3: rename hookMessage role to custom
Result<void> migrate_v2_to_v3(SessionFile& file);

// Auto-detect and migrate if needed
Result<void> migrate_if_needed(SessionFile& file);

}  // namespace pie::session
