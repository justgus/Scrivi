#include "RepairIssueJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

// ---------------------------------------------------------------------------
// Enum → string helpers
// ---------------------------------------------------------------------------

static std::string_view severityToString(RepairSeverity s) {
    switch (s) {
        case RepairSeverity::info:     return "info";
        case RepairSeverity::warning:  return "warning";
        case RepairSeverity::blocking: return "blocking";
    }
    return "info";
}

static RepairSeverity severityFromString(std::string_view s) {
    if (s == "warning")  return RepairSeverity::warning;
    if (s == "blocking") return RepairSeverity::blocking;
    return RepairSeverity::info;
}

static std::string_view categoryToString(RepairCategory c) {
    switch (c) {
        case RepairCategory::none:                      return "none";
        case RepairCategory::safeExternalEdit:          return "safeExternalEdit";
        case RepairCategory::unregisteredManuscriptFile: return "unregisteredManuscriptFile";
        case RepairCategory::missingContent:            return "missingContent";
        case RepairCategory::missingMetadata:           return "missingMetadata";
        case RepairCategory::possibleRename:            return "possibleRename";
        case RepairCategory::orphanMetadata:            return "orphanMetadata";
        case RepairCategory::corruptMetadata:           return "corruptMetadata";
        case RepairCategory::unsupportedSchema:         return "unsupportedSchema";
        case RepairCategory::gitStateChanged:           return "gitStateChanged";
        case RepairCategory::mergeConflict:             return "mergeConflict";
        case RepairCategory::unknownFile:               return "unknownFile";
        case RepairCategory::unknownIssue:              return "unknownIssue";
    }
    return "unknownIssue";
}

static RepairCategory categoryFromString(std::string_view s) {
    if (s == "safeExternalEdit")           return RepairCategory::safeExternalEdit;
    if (s == "unregisteredManuscriptFile") return RepairCategory::unregisteredManuscriptFile;
    if (s == "missingContent")             return RepairCategory::missingContent;
    if (s == "missingMetadata")            return RepairCategory::missingMetadata;
    if (s == "possibleRename")             return RepairCategory::possibleRename;
    if (s == "orphanMetadata")             return RepairCategory::orphanMetadata;
    if (s == "corruptMetadata")            return RepairCategory::corruptMetadata;
    if (s == "unsupportedSchema")          return RepairCategory::unsupportedSchema;
    if (s == "gitStateChanged")            return RepairCategory::gitStateChanged;
    if (s == "mergeConflict")              return RepairCategory::mergeConflict;
    if (s == "unknownFile")               return RepairCategory::unknownFile;
    return RepairCategory::unknownIssue;
}

static std::string_view actionKindToString(RepairActionKind k) {
    switch (k) {
        case RepairActionKind::none:                    return "none";
        case RepairActionKind::reloadExternalVersion:   return "reloadExternalVersion";
        case RepairActionKind::keepCurrentVersion:      return "keepCurrentVersion";
        case RepairActionKind::saveCurrentVersionAsCopy: return "saveCurrentVersionAsCopy";
        case RepairActionKind::importAsNewScene:        return "importAsNewScene";
        case RepairActionKind::attachToExistingScene:   return "attachToExistingScene";
        case RepairActionKind::regenerateMetadata:      return "regenerateMetadata";
        case RepairActionKind::restoreFromSnapshot:     return "restoreFromSnapshot";
        case RepairActionKind::createEmptyContentFile:  return "createEmptyContentFile";
        case RepairActionKind::relinkToFile:            return "relinkToFile";
        case RepairActionKind::markMissing:             return "markMissing";
        case RepairActionKind::removeFromProject:       return "removeFromProject";
        case RepairActionKind::moveToInbox:             return "moveToInbox";
        case RepairActionKind::ignore:                  return "ignore";
        case RepairActionKind::deleteAfterConfirmation: return "deleteAfterConfirmation";
        case RepairActionKind::openReadOnly:            return "openReadOnly";
        case RepairActionKind::cancelOpen:              return "cancelOpen";
    }
    return "none";
}

static RepairActionKind actionKindFromString(std::string_view s) {
    if (s == "reloadExternalVersion")    return RepairActionKind::reloadExternalVersion;
    if (s == "keepCurrentVersion")       return RepairActionKind::keepCurrentVersion;
    if (s == "saveCurrentVersionAsCopy") return RepairActionKind::saveCurrentVersionAsCopy;
    if (s == "importAsNewScene")         return RepairActionKind::importAsNewScene;
    if (s == "attachToExistingScene")    return RepairActionKind::attachToExistingScene;
    if (s == "regenerateMetadata")       return RepairActionKind::regenerateMetadata;
    if (s == "restoreFromSnapshot")      return RepairActionKind::restoreFromSnapshot;
    if (s == "createEmptyContentFile")   return RepairActionKind::createEmptyContentFile;
    if (s == "relinkToFile")             return RepairActionKind::relinkToFile;
    if (s == "markMissing")              return RepairActionKind::markMissing;
    if (s == "removeFromProject")        return RepairActionKind::removeFromProject;
    if (s == "moveToInbox")              return RepairActionKind::moveToInbox;
    if (s == "ignore")                   return RepairActionKind::ignore;
    if (s == "deleteAfterConfirmation")  return RepairActionKind::deleteAfterConfirmation;
    if (s == "openReadOnly")             return RepairActionKind::openReadOnly;
    if (s == "cancelOpen")               return RepairActionKind::cancelOpen;
    return RepairActionKind::none;
}

// ---------------------------------------------------------------------------
// Serialize
// ---------------------------------------------------------------------------

std::string serializeRepairIssues(const std::vector<RepairIssue>& issues) {
    util::JsonDoc root;
    root.setString("schema",       "scrivi-repair-issues");
    root.setInt("schemaVersion",   1);

    for (const auto& issue : issues) {
        util::JsonDoc doc;
        doc.setString("issueID",     issue.issueID);
        doc.setString("severity",    std::string(severityToString(issue.severity)));
        doc.setString("category",    std::string(categoryToString(issue.category)));
        doc.setString("title",       issue.title);
        doc.setString("message",     issue.message);
        doc.setString("path",        issue.path);
        doc.setString("relatedPath", issue.relatedPath);
        doc.setString("projectID",   issue.projectID.value);
        doc.setString("chapterID",   issue.chapterID.value);
        doc.setString("sceneID",     issue.sceneID.value);

        for (const auto& action : issue.suggestedActions) {
            util::JsonDoc actionDoc;
            actionDoc.setString("kind",   std::string(actionKindToString(action.kind)));
            actionDoc.setString("label",  action.label);
            actionDoc.setString("detail", action.detail);
            doc.appendToArray("suggestedActions", std::move(actionDoc));
        }

        root.appendToArray("issues", std::move(doc));
    }

    return root.dump();
}

// ---------------------------------------------------------------------------
// Parse
// ---------------------------------------------------------------------------

Result<std::vector<RepairIssue>> parseRepairIssues(std::string_view json) {
    auto r = util::parseJson(json);
    if (!r.ok()) return Result<std::vector<RepairIssue>>::failure(r.error());
    auto& root = r.value();

    auto schemaTag = root.getString("schema");
    if (schemaTag != "scrivi-repair-issues")
        return Result<std::vector<RepairIssue>>::failure(
            {ErrorCode::validationError, "unexpected schema: " + schemaTag});

    std::vector<RepairIssue> issues;
    const auto count = root.arraySize("issues");
    issues.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        auto doc = root.arrayItem("issues", i);
        RepairIssue issue;
        issue.issueID     = doc.getString("issueID");
        issue.severity    = severityFromString(doc.getString("severity"));
        issue.category    = categoryFromString(doc.getString("category"));
        issue.title       = doc.getString("title");
        issue.message     = doc.getString("message");
        issue.path        = doc.getString("path");
        issue.relatedPath = doc.getString("relatedPath");
        issue.projectID   = ProjectID{doc.getString("projectID")};
        issue.chapterID   = ChapterID{doc.getString("chapterID")};
        issue.sceneID     = SceneID{doc.getString("sceneID")};

        const auto actionCount = doc.arraySize("suggestedActions");
        issue.suggestedActions.reserve(actionCount);
        for (std::size_t j = 0; j < actionCount; ++j) {
            auto aDoc = doc.arrayItem("suggestedActions", j);
            RepairAction action;
            action.kind   = actionKindFromString(aDoc.getString("kind"));
            action.label  = aDoc.getString("label");
            action.detail = aDoc.getString("detail");
            issue.suggestedActions.push_back(std::move(action));
        }

        issues.push_back(std::move(issue));
    }

    return Result<std::vector<RepairIssue>>::success(std::move(issues));
}

} // namespace scrivi::schemas
