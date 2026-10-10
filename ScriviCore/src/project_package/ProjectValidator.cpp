#include "project_package/ProjectValidator.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <filesystem>
#include <unordered_set>
#include <optional>

namespace scrivi::project_package {

ProjectValidator::ProjectValidator(CoreServices& services)
    : services_(services) {}

Result<std::vector<RepairIssue>> ProjectValidator::validate(
    const AbsolutePath& projectRoot) const
{
    auto& fs = *services_.fileSystem;
    std::vector<RepairIssue> issues;

    // Check project.json exists
    auto projPath = util::join(projectRoot, "project.json");
    auto existsR  = fs.exists(projPath);
    if (!existsR.ok()) { return Result<std::vector<RepairIssue>>::failure(existsR.error());
}
    if (!existsR.value()) {
        RepairIssue issue;
        issue.severity = RepairSeverity::blocking;
        issue.category = RepairCategory::missingMetadata;
        issue.title    = "Missing project.json";
        issue.path     = projPath;
        issues.push_back(std::move(issue));
        return Result<std::vector<RepairIssue>>::success(std::move(issues));
    }

    // Read manuscript.meta.json
    auto msPath  = util::join(projectRoot, "manuscript/manuscript.meta.json");
    auto msExistsR = fs.exists(msPath);
    if (!msExistsR.ok()) { return Result<std::vector<RepairIssue>>::failure(msExistsR.error());
}
    if (!msExistsR.value()) {
        RepairIssue issue;
        issue.severity = RepairSeverity::blocking;
        issue.category = RepairCategory::missingMetadata;
        issue.title    = "Missing manuscript.meta.json";
        issue.path     = msPath;
        issues.push_back(std::move(issue));
        return Result<std::vector<RepairIssue>>::success(std::move(issues));
    }

    auto msTextR = fs.readTextFile(msPath);
    if (!msTextR.ok()) { return Result<std::vector<RepairIssue>>::failure(msTextR.error());
}

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) {
        RepairIssue issue;
        issue.severity = RepairSeverity::blocking;
        issue.category = RepairCategory::corruptMetadata;
        issue.title    = "Corrupt manuscript.meta.json";
        issue.path     = msPath;
        issue.message  = msParsed.error().message;
        issues.push_back(std::move(issue));
        return Result<std::vector<RepairIssue>>::success(std::move(issues));
    }

    // For each chapter, check chapter.meta.json and scene files
    for (auto& chRef : msParsed.value().chapters) {
        auto chPath   = util::join(projectRoot, chRef.path);
        auto chExistsR = fs.exists(chPath);
        if (!chExistsR.ok()) { return Result<std::vector<RepairIssue>>::failure(chExistsR.error());
}

        if (!chExistsR.value()) {
            RepairIssue issue;
            issue.severity  = RepairSeverity::blocking;
            issue.category  = RepairCategory::missingMetadata;
            issue.title     = "Missing chapter.meta.json";
            issue.path      = chPath;
            issue.chapterID = chRef.chapterID;
            issues.push_back(std::move(issue));
            continue;
        }

        auto chTextR = fs.readTextFile(chPath);
        if (!chTextR.ok()) { return Result<std::vector<RepairIssue>>::failure(chTextR.error());
}

        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) {
            RepairIssue issue;
            issue.severity  = RepairSeverity::blocking;
            issue.category  = RepairCategory::corruptMetadata;
            issue.title     = "Corrupt chapter.meta.json";
            issue.path      = chPath;
            issue.chapterID = chRef.chapterID;
            issue.message   = chParsed.error().message;
            issues.push_back(std::move(issue));
            continue;
        }

        // EP-027 §8.1: scene refs are BARE FILENAMES resolved against the chapter's own
        // folder; scene identity comes from the sidecar (not the ref). The chapter dir is
        // the parent of its chapter.meta.json.
        const std::string chapterDir =
            std::filesystem::path(chRef.path).parent_path().string();

        // SP-173 (I-0285 AC3, item 4): ONE listing of the chapter's folder answers "does this file exist?" for every scene
        // in it. ⛔ Measured on the rig (dumas over the share): a stat per scene text file cost ~7–9 s of the open — every
        // one a network round trip. Compared by FILE NAME (no path-spelling mismatch). If the listing fails, each check
        // falls back to `exists`, so a missing file is still reported exactly as before.
        std::optional<std::unordered_set<std::string>> namesInDir;
        if (auto listR = fs.listDirectory(util::join(projectRoot, chapterDir)); listR.ok()) {
            namesInDir.emplace();
            for (const auto& entry : listR.value()) {
                namesInDir->insert(std::filesystem::path(entry).filename().string());
            }
        }
        auto existsInChapter = [&](const AbsolutePath& path) -> Result<bool> {
            if (namesInDir) {
                return Result<bool>::success(namesInDir->count(std::filesystem::path(path).filename().string()) > 0);
            }
            return fs.exists(path);
        };

        for (auto& scRef : chParsed.value().scenes) {
            const std::string sMetaRel = chapterDir + "/" + scRef.metadataFilename;
            auto sMetaPath  = util::join(projectRoot, sMetaRel);
            auto sMetaExistsR = existsInChapter(sMetaPath);
            if (!sMetaExistsR.ok()) {
                return Result<std::vector<RepairIssue>>::failure(sMetaExistsR.error());
}

            if (!sMetaExistsR.value()) {
                RepairIssue issue;
                issue.severity  = RepairSeverity::blocking;
                issue.category  = RepairCategory::missingMetadata;
                issue.title     = "Missing scene.meta.json";
                issue.path      = sMetaPath;
                issue.chapterID = chRef.chapterID;
                // sceneID unknown (the ref no longer carries it and the sidecar is missing).
                issues.push_back(std::move(issue));
                continue;
            }

            auto sTextR = fs.readTextFile(sMetaPath);
            if (!sTextR.ok()) { return Result<std::vector<RepairIssue>>::failure(sTextR.error());
}

            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) {
                RepairIssue issue;
                issue.severity  = RepairSeverity::blocking;
                issue.category  = RepairCategory::corruptMetadata;
                issue.title     = "Corrupt scene.meta.json";
                issue.path      = sMetaPath;
                issue.chapterID = chRef.chapterID;
                issue.message   = sParsed.error().message;
                issues.push_back(std::move(issue));
                continue;
            }

            // Check content file exists — contentPath is a bare filename (§8.1).
            auto contentPath   =
                util::join(projectRoot, chapterDir + "/" + sParsed.value().contentPath);
            auto contentExistsR = existsInChapter(contentPath);
            if (!contentExistsR.ok()) {
                return Result<std::vector<RepairIssue>>::failure(contentExistsR.error());
}

            if (!contentExistsR.value()) {
                RepairIssue issue;
                issue.severity  = RepairSeverity::blocking;
                issue.category  = RepairCategory::missingContent;
                issue.title     = "Missing scene content file";
                issue.path      = contentPath;
                issue.chapterID = chRef.chapterID;
                issue.sceneID   = sParsed.value().sceneID;
                issues.push_back(std::move(issue));
            }
        }
    }

    return Result<std::vector<RepairIssue>>::success(std::move(issues));
}

} // namespace scrivi::project_package
