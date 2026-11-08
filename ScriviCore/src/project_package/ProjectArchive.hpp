#pragma once
// ProjectArchive — one project, and optionally its worlds, in one `.scrivarchive` zip (EP-054 S1, SP-205).
//
// An archive is NEVER a project: nothing reads, searches or edits inside it. It is written by `exportArchive` and
// unpacked by `importArchive`; the platform then opens the unpacked project the ordinary way.
//
// Rulings (EP-054, user 2026-11-06): Q4 a project folder name already in the chosen folder is Cancel / Rename /
// Replace and the project KEEPS ITS ID; Q8 + SP-205 Q1 a world already present (same world ID) is bound, not
// unpacked, found by asking the filesystem, not a list; SP-205 Q3 a world name clash is Cancel / Rename only;
// Q9 what is left out.

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::project_package {

struct ArchivedWorld {
    std::string worldID;
    std::string folder;   // the world's top-level folder inside the archive (its package name)
};

struct SkippedWorld {
    std::string worldID;
    std::string reason;   // why it could not be carried (the resolution status)
};

struct ArchiveExportResult {
    AbsolutePath               archivePath;
    std::int64_t               files = 0;
    std::vector<ArchivedWorld> worlds;
    std::vector<SkippedWorld>  skippedWorlds;
};

struct ArchiveImportOptions {
    std::string projectFolderName;                       // Rename; empty = the archive's own name
    bool        replaceProject = false;                  // Replace (Q4)
    std::map<std::string, std::string> worldFolderNames; // worldID → Rename (SP-205 Q3: no Replace for worlds)
    // Replace is refused while the project it would delete is open.
    std::function<bool(const AbsolutePath&)> isProjectOpen;
};

struct ImportedWorld {
    std::string  worldID;
    AbsolutePath packagePath;
    bool         unpacked = false;   // false: a world already here was bound instead
};

// A name already in the chosen folder: nothing was extracted; the platform asks the writer and calls again.
struct ArchiveConflict {
    std::string kind;          // "project" | "world"
    std::string folderName;
    std::string worldID;       // kind == "world"
};

struct ArchiveImportResult {
    bool                       imported = false;
    ArchiveConflict            conflict;   // set when !imported
    AbsolutePath               projectRoot;
    std::string                projectID;
    std::vector<ImportedWorld> worlds;
};

class ProjectArchive {
public:
    explicit ProjectArchive(CoreServices& services) : services_(services) {}

    [[nodiscard]] Result<ArchiveExportResult> exportArchive(const AbsolutePath& projectRoot,
                                                            const AbsolutePath& archivePath,
                                                            bool includeWorlds) const;

    [[nodiscard]] Result<ArchiveImportResult> importArchive(const AbsolutePath& archivePath,
                                                            const AbsolutePath& destFolder,
                                                            const ArchiveImportOptions& options) const;

    // Q9: true when a path inside a package (relative, '/'-separated) is left out of the archive.
    [[nodiscard]] static bool excluded(std::string_view relativePath);

private:
    CoreServices& services_;
};

} // namespace scrivi::project_package
