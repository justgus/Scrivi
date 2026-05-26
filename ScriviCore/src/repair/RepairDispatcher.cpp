#include "RepairDispatcher.hpp"

#include "repair/ExternalChangeScanner.hpp"

#include <algorithm>

namespace scrivi::repair {

RepairDispatcher::RepairDispatcher(CoreServices services)
    : services_(services) {}

// ---------------------------------------------------------------------------
// Stub handlers — to be replaced by RepairHandlers in T-0030
// ---------------------------------------------------------------------------

static Result<ApplyRepairResult> stubNotImplemented(
    const ApplyRepairRequest& request, std::string_view handlerName)
{
    return Result<ApplyRepairResult>::failure({
        ErrorCode::internalError,
        std::string(handlerName) + " handler not yet implemented",
        request.projectRootPath,
        "scheduled for T-0030"
    });
}

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

    // Dispatch to the appropriate handler (T-0030 will replace stubs)
    switch (request.actionKind) {
        case RepairActionKind::relinkToFile:
            return stubNotImplemented(request, "relinkToFile");

        case RepairActionKind::createEmptyContentFile:
            return stubNotImplemented(request, "createEmptyContentFile");

        case RepairActionKind::markMissing:
            return stubNotImplemented(request, "markMissing");

        case RepairActionKind::removeFromProject:
            return stubNotImplemented(request, "removeFromProject");

        case RepairActionKind::moveToInbox:
            return stubNotImplemented(request, "moveToInbox");

        case RepairActionKind::reloadExternalVersion:
            return stubNotImplemented(request, "reloadExternalVersion");

        case RepairActionKind::regenerateMetadata:
            return stubNotImplemented(request, "regenerateMetadata");

        case RepairActionKind::importAsNewScene:
            return stubNotImplemented(request, "importAsNewScene");

        case RepairActionKind::attachToExistingScene:
            return stubNotImplemented(request, "attachToExistingScene");

        case RepairActionKind::keepCurrentVersion:
            return stubNotImplemented(request, "keepCurrentVersion");

        case RepairActionKind::saveCurrentVersionAsCopy:
            return stubNotImplemented(request, "saveCurrentVersionAsCopy");

        case RepairActionKind::restoreFromSnapshot:
            return stubNotImplemented(request, "restoreFromSnapshot");

        case RepairActionKind::ignore:
            return stubNotImplemented(request, "ignore");

        case RepairActionKind::deleteAfterConfirmation:
            return stubNotImplemented(request, "deleteAfterConfirmation");

        case RepairActionKind::openReadOnly:
            return stubNotImplemented(request, "openReadOnly");

        case RepairActionKind::cancelOpen:
            return stubNotImplemented(request, "cancelOpen");

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
