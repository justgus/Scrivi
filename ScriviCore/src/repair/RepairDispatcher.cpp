#include "RepairDispatcher.hpp"

#include "repair/ExternalChangeScanner.hpp"
#include "repair/RepairHandlers.hpp"

#include <algorithm>

namespace scrivi::repair {

RepairDispatcher::RepairDispatcher(CoreServices services)
    : services_(services) {}

// ---------------------------------------------------------------------------
// apply — re-scan to validate issue exists, then dispatch
// ---------------------------------------------------------------------------

Result<ApplyRepairResult> RepairDispatcher::apply(const ApplyRepairRequest& request) {
    // Validate inputs
    if (request.issueID.empty())
        return Result<ApplyRepairResult>::failure(
            {ErrorCode::invalidArgument, "issueID must not be empty"});

    if (request.actionKind == RepairActionKind::none)
        return Result<ApplyRepairResult>::failure(
            {ErrorCode::invalidArgument, "actionKind must not be none"});

    // Re-read project state: re-scan to verify the issue still applies
    ExternalChangeScanRequest scanRequest;
    scanRequest.projectRootPath = request.projectRootPath;
    scanRequest.appSupportRoot  = request.appSupportRoot;
    scanRequest.includeGitStatus = false;   // no git check needed for repair validation

    ExternalChangeScanner scanner{services_};
    auto scanResult = scanner.scan(scanRequest);
    if (!scanResult.ok())
        return Result<ApplyRepairResult>::failure(scanResult.error());

    // Find the matching issue by ID
    const auto& issues = scanResult.value().repairIssues;
    auto it = std::find_if(issues.begin(), issues.end(),
        [&](const RepairIssue& i) { return i.issueID == request.issueID; });

    if (it == issues.end())
        return Result<ApplyRepairResult>::failure({
            ErrorCode::invalidArgument,
            "issue '" + request.issueID + "' not found in current project scan — "
            "it may have already been resolved"
        });

    const RepairIssue& issue = *it;

    // Verify the requested actionKind is in the issue's suggestedActions
    bool actionOffered = std::any_of(
        issue.suggestedActions.begin(), issue.suggestedActions.end(),
        [&](const RepairAction& a) { return a.kind == request.actionKind; });

    if (!actionOffered)
        return Result<ApplyRepairResult>::failure({
            ErrorCode::invalidArgument,
            "actionKind is not a suggested action for issue '" + request.issueID + "'"
        });

    // Build a HandlerContext for the real handlers
    HandlerContext hctx{request, issue, services_};

    // Dispatch to the appropriate handler
    switch (request.actionKind) {
        // --- T-0030 implemented handlers ---
        case RepairActionKind::relinkToFile:
            return handleRelinkToFile(hctx);

        case RepairActionKind::createEmptyContentFile:
            return handleCreateEmptyContentFile(hctx);

        case RepairActionKind::markMissing:
            return handleMarkMissing(hctx);

        case RepairActionKind::removeFromProject:
            return handleRemoveFromProject(hctx);

        case RepairActionKind::moveToInbox:
            return handleMoveToInbox(hctx);

        case RepairActionKind::reloadExternalVersion:
            return handleReloadExternalVersion(hctx);

        case RepairActionKind::regenerateMetadata:
            return handleRegenerateMetadata(hctx);

        // --- Not-yet-implemented action kinds (out of T-0030 scope) ---
        case RepairActionKind::importAsNewScene:
        case RepairActionKind::attachToExistingScene:
        case RepairActionKind::keepCurrentVersion:
        case RepairActionKind::saveCurrentVersionAsCopy:
        case RepairActionKind::restoreFromSnapshot:
        case RepairActionKind::ignore:
        case RepairActionKind::deleteAfterConfirmation:
        case RepairActionKind::openReadOnly:
        case RepairActionKind::cancelOpen:
            return Result<ApplyRepairResult>::failure({
                ErrorCode::internalError,
                "handler not yet implemented for this actionKind",
                request.projectRootPath
            });

        case RepairActionKind::none:
            // Already rejected above, but required for exhaustive switch
            return Result<ApplyRepairResult>::failure(
                {ErrorCode::invalidArgument, "actionKind must not be none"});
    }

    // Unreachable — silence compiler warning
    return Result<ApplyRepairResult>::failure(
        {ErrorCode::internalError, "unhandled actionKind in dispatch"});
}

} // namespace scrivi::repair
