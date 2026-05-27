#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::repair {

// ---------------------------------------------------------------------------
// Individual repair handler functions, one per RepairActionKind.
// Each handler receives the full request, the current scan result for the
// issue being repaired (already validated by the dispatcher), and services.
// Handlers are stateless: they read what they need from disk, act, and return.
//
// Backup convention: handlers that overwrite existing metadata create a
// .bak copy of the original before writing. The .bak is not tracked or
// cleaned up automatically — it serves as a last-resort manual recovery path.
// ---------------------------------------------------------------------------

struct HandlerContext {
    const ApplyRepairRequest& request;
    const RepairIssue&        issue;
    CoreServices&             services;
};

// --- Manuscript content handlers ---

// relinkToFile: update scene metadata contentPath to point at request.targetPath.
Result<ApplyRepairResult> handleRelinkToFile(const HandlerContext& ctx);

// createEmptyContentFile: write an empty .md at the missing content path.
Result<ApplyRepairResult> handleCreateEmptyContentFile(const HandlerContext& ctx);

// markMissing: set missingContent flag in scene metadata.
Result<ApplyRepairResult> handleMarkMissing(const HandlerContext& ctx);

// removeFromProject: remove the scene entry from its chapter metadata.
Result<ApplyRepairResult> handleRemoveFromProject(const HandlerContext& ctx);

// moveToInbox: move the file at issue.path to projectRoot/inbox/dropped-files/.
Result<ApplyRepairResult> handleMoveToInbox(const HandlerContext& ctx);

// reloadExternalVersion: no write — returns current on-disk content in detail field.
Result<ApplyRepairResult> handleReloadExternalVersion(const HandlerContext& ctx);

// regenerateMetadata (scene or chapter): reconstruct .meta.json from file/folder.
Result<ApplyRepairResult> handleRegenerateMetadata(const HandlerContext& ctx);

} // namespace scrivi::repair
