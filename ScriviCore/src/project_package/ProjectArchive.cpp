#include "project_package/ProjectArchive.hpp"

#include "schemas/ProjectJson.hpp"
#include "schemas/WorldJson.hpp"
#include "util/Json.hpp"
#include "util/PathUtils.hpp"
#include "worlds/WorldStore.hpp"

#include "miniz.h"

#include <algorithm>
#include <filesystem>
#include <random>
#include <set>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

namespace scrivi::project_package {

namespace {

// The archive's own table of contents, its first entry. A zip without it is not a Scrivi archive.
constexpr const char* kManifestName = "scrivarchive.json";
constexpr std::string_view kFormat  = "scrivi.archive.v1";
constexpr std::string_view kProjectExt = ".scrivi";
constexpr std::string_view kWorldExt   = ".scrivworld";

Error ioError(std::string message, const AbsolutePath& path) {
    return {.code = ErrorCode::ioError, .message = std::move(message), .path = path};
}
Error invalid(std::string message, const AbsolutePath& path = {}, std::string detail = {}) {
    return {.code = ErrorCode::invalidArgument, .message = std::move(message), .path = path,
            .detail = std::move(detail)};
}

// A folder path without its trailing separator: the Mac passes the project as `…/Novel.scrivi/`, whose
// filename() is empty, and an empty prefix makes every entry name absolute (miniz: "invalid filename").
fs::path folderPath(const std::string& p) {
    auto q = fs::path(p).lexically_normal();
    return q.has_filename() ? q : q.parent_path();
}

std::string zipError(mz_zip_archive& z) { return mz_zip_get_error_string(mz_zip_get_last_error(&z)); }

struct ZipWriter {
    mz_zip_archive z{};
    bool open = false;
    ~ZipWriter() { if (open) { mz_zip_writer_end(&z); } }
};
struct ZipReader {
    mz_zip_archive z{};
    bool open = false;
    ~ZipReader() { if (open) { mz_zip_reader_end(&z); } }
};

std::string uniqueSuffix() {
    std::random_device rd;
    std::ostringstream s;
    s << std::hex << rd() << rd();
    return s.str();
}

// The projectID in `root`/project.json.
Result<std::string> projectIDAt(FileSystem& files, const AbsolutePath& root) {
    auto text = files.readTextFile(util::join(root, "project.json"));
    if (!text.ok()) { return Result<std::string>::failure(text.error()); }
    auto parsed = schemas::parseProject(text.value());
    if (!parsed.ok()) { return Result<std::string>::failure(parsed.error()); }
    return Result<std::string>::success(parsed.value().projectID.value);
}

// The worldID of the package at `path`, or "" when there is no readable world there.
std::string worldIDAt(FileSystem& files, const AbsolutePath& path) {
    auto text = files.readTextFile(worlds::WorldStore::worldJsonPath(path));
    if (!text.ok()) { return {}; }
    auto parsed = schemas::parseWorld(text.value());
    return parsed.ok() ? parsed.value().worldID : std::string{};
}

// Adds every file and folder under `root` as `prefix/<relative>`, less Q9's list. Symlinks are not followed.
Result<std::int64_t> addTree(mz_zip_archive& z, const fs::path& root, const std::string& prefix) {
    std::int64_t files = 0;
    std::error_code ec;
    if (!mz_zip_writer_add_mem(&z, (prefix + "/").c_str(), "", 0, 0)) {
        return Result<std::int64_t>::failure(ioError("could not write to the archive: " + zipError(z), root.string()));
    }
    fs::recursive_directory_iterator it(root, ec), end;
    if (ec) { return Result<std::int64_t>::failure(ioError("could not read " + ec.message(), root.string())); }
    for (; it != end; it.increment(ec)) {
        if (ec) { return Result<std::int64_t>::failure(ioError("could not read " + ec.message(), root.string())); }
        const auto rel = it->path().lexically_relative(root).generic_string();
        const auto status = it->symlink_status(ec);
        if (ec) { return Result<std::int64_t>::failure(ioError("could not read " + ec.message(), it->path().string())); }
        if (ProjectArchive::excluded(rel) || fs::is_symlink(status)) {
            if (fs::is_directory(status)) { it.disable_recursion_pending(); }
            continue;
        }
        const auto name = prefix + "/" + rel;
        bool added = true;
        if (fs::is_directory(status)) {
            added = mz_zip_writer_add_mem(&z, (name + "/").c_str(), "", 0, 0);   // "" not nullptr: miniz fwrite()s it, and UBSan rejects a null fwrite
        } else if (fs::is_regular_file(status)) {
            added = mz_zip_writer_add_file(&z, name.c_str(), it->path().string().c_str(), nullptr, 0,
                                           MZ_DEFAULT_LEVEL);
            ++files;
        }
        if (!added) {
            return Result<std::int64_t>::failure(
                ioError("could not add a file to the archive: " + zipError(z), it->path().string()));
        }
    }
    return Result<std::int64_t>::success(files);
}

// An entry name is safe when it stays inside the folder it is unpacked into: relative, no `..`, no drive or
// backslash, no empty component.
bool safeEntryName(std::string_view name) {
    if (name.empty() || name.front() == '/' || name.find('\\') != std::string_view::npos ||
        name.find(':') != std::string_view::npos) {
        return false;
    }
    std::string_view rest = name;
    if (rest.back() == '/') { rest.remove_suffix(1); }
    while (!rest.empty()) {
        const auto slash = rest.find('/');
        const auto part = rest.substr(0, slash);
        if (part.empty() || part == "." || part == "..") { return false; }
        if (slash == std::string_view::npos) { break; }
        rest.remove_prefix(slash + 1);
    }
    return true;
}

// A folder name the writer typed for Rename: one path component, given the package's extension if it lacks it.
Result<std::string> folderName(std::string name, std::string_view ext) {
    if (name.empty() || name == "." || name == ".." || name.find('/') != std::string::npos ||
        name.find('\\') != std::string::npos || util::containsControlCharacter(name)) {
        return Result<std::string>::failure(invalid("not a usable folder name: " + name, {}, "badFolderName"));
    }
    if (!name.ends_with(ext)) { name += ext; }
    return Result<std::string>::success(std::move(name));
}

Result<std::string> readEntry(mz_zip_archive& z, const std::string& name) {
    std::size_t size = 0;
    void* data = mz_zip_reader_extract_file_to_heap(&z, name.c_str(), &size, 0);
    if (data == nullptr) {
        return Result<std::string>::failure(invalid("the archive has no " + name, {}, "notAnArchive"));
    }
    std::string out(static_cast<const char*>(data), size);
    mz_free(data);
    return Result<std::string>::success(std::move(out));
}

} // namespace

bool ProjectArchive::excluded(std::string_view rel) {
    // Q9 — the package spec's §17 list: rebuilt on open, or never content.
    const auto slash = rel.rfind('/');
    const auto name  = slash == std::string_view::npos ? rel : rel.substr(slash + 1);
    const bool atRoot = rel.find('/') == std::string_view::npos;
    if (atRoot && (name == ".cache" || name == "indexes")) { return true; }
    if (util::isIgnorableFilesystemArtifact(name)) { return true; }
    for (std::string_view suffix : {".lock", ".tmp", ".swp", ".sqlite", ".sqlite-shm", ".sqlite-wal"}) {
        if (name.ends_with(suffix)) { return true; }
    }
    return name.starts_with("~$");
}

// ---------------------------------------------------------------------------
// Export
// ---------------------------------------------------------------------------

Result<ArchiveExportResult> ProjectArchive::exportArchive(const AbsolutePath& projectRoot,
                                                          const AbsolutePath& archivePath,
                                                          bool includeWorlds) const {
    using R = Result<ArchiveExportResult>;
    auto& files = *services_.fileSystem;

    auto projectID = projectIDAt(files, projectRoot);
    if (!projectID.ok()) { return R::failure(projectID.error()); }

    const fs::path root = folderPath(projectRoot);
    const fs::path dest = fs::path(archivePath).lexically_normal();
    const auto inside = dest.lexically_relative(root);
    if (!inside.empty() && *inside.begin() != "..") {
        return R::failure(invalid("an archive cannot be written inside the project it holds", archivePath));
    }

    ArchiveExportResult out;
    out.archivePath = archivePath;
    std::vector<std::pair<std::string, fs::path>> worldTrees;   // archive folder → package
    if (includeWorlds) {
        worlds::WorldStore store{services_};
        // listWorlds, not the bare folder list: a removed reference leaves its folder behind with no binding (I-0261).
        auto bound = store.listWorlds(projectRoot);
        if (!bound.ok()) { return R::failure(bound.error()); }
        std::set<std::string> folders;
        for (const auto& w : bound.value()) {
            if (w.status != worlds::WorldStatus::available) {
                out.skippedWorlds.push_back({w.worldID, worlds::worldStatusName(w.status)});
                continue;
            }
            const auto package = folderPath(w.packagePath);
            auto folder = package.filename().string();
            if (!folders.insert(folder).second) {
                return R::failure(invalid("two of the project's worlds have the same folder name: " + folder,
                                          w.packagePath));
            }
            out.worlds.push_back({w.worldID, folder});
            worldTrees.emplace_back(folder, package);
        }
    }

    // Written beside the destination and renamed into place, so a failed export leaves no file at `archivePath`.
    const auto partial = archivePath + "." + uniqueSuffix() + ".tmp";
    {
        ZipWriter w;
        if (!mz_zip_writer_init_file_v2(&w.z, partial.c_str(), 0, MZ_ZIP_FLAG_WRITE_ZIP64)) {
            return R::failure(ioError("could not create the archive: " + zipError(w.z), archivePath));
        }
        w.open = true;
        auto fail = [&](Error e) {
            mz_zip_writer_end(&w.z);
            w.open = false;
            std::error_code ec;
            fs::remove(partial, ec);
            return R::failure(std::move(e));
        };

        const auto projectFolder = root.filename().string();
        util::JsonDoc manifest;
        manifest.setString("format", kFormat);
        manifest.setString("projectFolder", projectFolder);
        manifest.setString("projectID", projectID.value());
        for (const auto& w2 : out.worlds) {
            util::JsonDoc item;
            item.setString("worldID", w2.worldID);
            item.setString("folder", w2.folder);
            manifest.appendToArray("worlds", std::move(item));
        }
        const auto text = manifest.dump();
        if (!mz_zip_writer_add_mem(&w.z, kManifestName, text.data(), text.size(), MZ_DEFAULT_LEVEL)) {
            return fail(ioError("could not write to the archive: " + zipError(w.z), archivePath));
        }

        auto added = addTree(w.z, root, projectFolder);
        if (!added.ok()) { return fail(added.error()); }
        out.files = added.value();
        for (const auto& [folder, package] : worldTrees) {
            auto worldAdded = addTree(w.z, package, folder);
            if (!worldAdded.ok()) { return fail(worldAdded.error()); }
            out.files += worldAdded.value();
        }

        if (!mz_zip_writer_finalize_archive(&w.z)) {
            return fail(ioError("could not finish the archive: " + zipError(w.z), archivePath));
        }
        mz_zip_writer_end(&w.z);
        w.open = false;
    }

    std::error_code ec;
    fs::rename(partial, archivePath, ec);
    if (ec) {
        fs::remove(partial, ec);
        return R::failure(ioError("could not move the archive into place", archivePath));
    }
    return R::success(std::move(out));
}

// ---------------------------------------------------------------------------
// Import
// ---------------------------------------------------------------------------

Result<ArchiveImportResult> ProjectArchive::importArchive(const AbsolutePath& archivePath,
                                                          const AbsolutePath& destFolder,
                                                          const ArchiveImportOptions& options) const {
    using R = Result<ArchiveImportResult>;
    auto& files = *services_.fileSystem;
    std::error_code ec;

    if (!fs::is_directory(destFolder, ec)) {
        return R::failure(invalid("the folder to unpack into does not exist", destFolder));
    }

    ZipReader r;
    if (!mz_zip_reader_init_file(&r.z, archivePath.c_str(), 0)) {
        return R::failure(invalid("not a Scrivi archive: " + zipError(r.z), archivePath, "notAnArchive"));
    }
    r.open = true;

    // --- the manifest, and every entry checked before anything is written ---
    auto manifestText = readEntry(r.z, kManifestName);
    if (!manifestText.ok()) { return R::failure(invalid("not a Scrivi archive", archivePath, "notAnArchive")); }
    auto manifest = util::parseJson(manifestText.value());
    if (!manifest.ok() || manifest.value().getString("format") != kFormat) {
        return R::failure(invalid("not a Scrivi archive this version can read", archivePath, "notAnArchive"));
    }
    const auto& m = manifest.value();
    const auto archivedProjectFolder = m.getString("projectFolder");
    const auto projectID = m.getString("projectID");
    if (!safeEntryName(archivedProjectFolder) || archivedProjectFolder.find('/') != std::string::npos ||
        projectID.empty()) {
        return R::failure(invalid("the archive's table of contents is damaged", archivePath, "notAnArchive"));
    }
    struct World { std::string id, archived, folder; AbsolutePath existing; };
    std::vector<World> worldList;
    std::map<std::string, std::string> topFolders{{archivedProjectFolder, ""}};   // archived folder → worldID
    for (std::size_t i = 0; i < m.arraySize("worlds"); ++i) {
        auto item = m.arrayItem("worlds", i);
        World w{item.getString("worldID"), item.getString("folder"), {}, {}};
        if (w.id.empty() || !safeEntryName(w.archived) || w.archived.find('/') != std::string::npos ||
            !topFolders.emplace(w.archived, w.id).second) {
            return R::failure(invalid("the archive's table of contents is damaged", archivePath, "notAnArchive"));
        }
        worldList.push_back(std::move(w));
    }
    const auto entries = mz_zip_reader_get_num_files(&r.z);
    for (mz_uint i = 0; i < entries; ++i) {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&r.z, i, &st)) {
            return R::failure(invalid("the archive is damaged: " + zipError(r.z), archivePath, "notAnArchive"));
        }
        const std::string name = st.m_filename;
        if (name == kManifestName) { continue; }
        const auto top = name.substr(0, name.find('/'));
        if (!safeEntryName(name) || !topFolders.contains(top)) {
            return R::failure(invalid("the archive holds a path outside its project and worlds: " + name,
                                      archivePath, "unsafeEntry"));
        }
    }

    // --- the names it will take here (Q4, SP-205 Q1, Q3), decided before anything is extracted ---
    auto projectFolder = options.projectFolderName.empty()
        ? Result<std::string>::success(archivedProjectFolder)
        : folderName(options.projectFolderName, kProjectExt);
    if (!projectFolder.ok()) { return R::failure(projectFolder.error()); }
    const fs::path dest = fs::path(destFolder);
    const fs::path target = dest / projectFolder.value();
    const bool replacing = fs::exists(target, ec);
    if (replacing) {
        if (!options.replaceProject) {
            ArchiveImportResult out;
            out.conflict = {"project", projectFolder.value(), {}};
            return R::success(std::move(out));
        }
        if (projectIDAt(files, target.string()).ok() == false) {
            return R::failure(invalid("Replace only replaces a Scrivi project; that folder is not one",
                                      target.string(), "notAProject"));
        }
        if (options.isProjectOpen && options.isProjectOpen(target.string())) {
            return R::failure(invalid("close the project before replacing it", target.string(), "projectOpen"));
        }
    }

    for (auto& w : worldList) {
        auto renamed = options.worldFolderNames.find(w.id);
        auto folder = renamed == options.worldFolderNames.end()
            ? Result<std::string>::success(w.archived)
            : folderName(renamed->second, kWorldExt);
        if (!folder.ok()) { return R::failure(folder.error()); }
        w.folder = folder.value();

        // Already here? Where the binding last saw it, then the chosen folder (Q8).
        std::vector<fs::path> candidates;
        auto binding = readEntry(r.z, archivedProjectFolder + "/worlds/" + w.id + "/binding.json");
        if (binding.ok()) {
            if (auto b = schemas::parseWorldBinding(binding.value()); b.ok()) {
                if (!b.value().reference.lastKnownAbsolutePath.empty()) {
                    candidates.emplace_back(b.value().reference.lastKnownAbsolutePath);
                }
                if (!b.value().reference.lastKnownPath.empty()) {
                    candidates.push_back((target / b.value().reference.lastKnownPath).lexically_normal());
                }
            }
        }
        candidates.push_back(dest / w.folder);
        for (const auto& c : candidates) {
            if (worldIDAt(files, c.string()) == w.id) { w.existing = c.string(); break; }
        }
        if (w.existing.empty() && fs::exists(dest / w.folder, ec)) {
            ArchiveImportResult out;
            out.conflict = {"world", w.folder, w.id};
            return R::success(std::move(out));
        }
    }

    // --- unpack into a temporary folder beside the destination, check it, then move it into place ---
    const fs::path staging = dest / (".scrivi-import-" + uniqueSuffix());
    auto cleanup = [&] { std::error_code ignored; fs::remove_all(staging, ignored); };
    fs::create_directories(staging, ec);
    if (ec) { return R::failure(ioError("could not create a folder to unpack into", staging.string())); }

    for (mz_uint i = 0; i < entries; ++i) {
        mz_zip_archive_file_stat st;
        mz_zip_reader_file_stat(&r.z, i, &st);
        const std::string name = st.m_filename;
        if (name == kManifestName) { continue; }
        const auto top = name.substr(0, name.find('/'));
        const auto& worldID = topFolders.at(top);
        if (!worldID.empty()) {
            auto w = std::find_if(worldList.begin(), worldList.end(), [&](auto& x) { return x.id == worldID; });
            if (!w->existing.empty()) { continue; }   // bound to the one already here: nothing unpacked
        }
        const fs::path out = staging / fs::path(name);
        if (mz_zip_reader_is_file_a_directory(&r.z, i)) {
            fs::create_directories(out, ec);
        } else {
            fs::create_directories(out.parent_path(), ec);
            if (!ec && !mz_zip_reader_extract_to_file(&r.z, i, out.string().c_str(), 0)) {
                cleanup();
                return R::failure(ioError("could not unpack " + name + ": " + zipError(r.z), archivePath));
            }
        }
        if (ec) {
            cleanup();
            return R::failure(ioError("could not unpack " + name + ": " + ec.message(), staging.string()));
        }
    }

    const fs::path stagedProject = staging / archivedProjectFolder;
    auto stagedID = projectIDAt(files, stagedProject.string());
    if (!stagedID.ok() || stagedID.value() != projectID) {
        cleanup();
        return R::failure(invalid("the archive's project is damaged (its project.json does not match)",
                                  archivePath, "notAnArchive"));
    }
    for (const auto& w : worldList) {
        if (w.existing.empty() && worldIDAt(files, (staging / w.archived).string()) != w.id) {
            cleanup();
            return R::failure(invalid("the archive's world " + w.archived + " is damaged", archivePath,
                                      "notAnArchive"));
        }
    }

    // New worlds first: they take names nothing holds (checked above), so the old project is touched last.
    std::vector<AbsolutePath> packages;
    for (const auto& w : worldList) {
        AbsolutePath package = w.existing;
        if (package.empty()) {
            package = (dest / w.folder).string();
            fs::rename(staging / w.archived, package, ec);
            if (ec) {
                cleanup();
                return R::failure(ioError("could not move the world " + w.folder + " into place: " + ec.message(),
                                          package));
            }
        }
        packages.push_back(std::move(package));
    }

    // Replace: the old project is set aside, the new one moved in, and only then is the old one deleted.
    const fs::path setAside = dest / (".scrivi-replaced-" + uniqueSuffix());
    if (replacing) {
        fs::rename(target, setAside, ec);
        if (ec) {
            cleanup();
            return R::failure(ioError("could not move the old project aside: " + ec.message(), target.string()));
        }
    }
    fs::rename(stagedProject, target, ec);
    if (ec) {
        std::error_code ignored;
        if (replacing) { fs::rename(setAside, target, ignored); }
        cleanup();
        return R::failure(ioError("could not move the project into place: " + ec.message(), target.string()));
    }
    if (replacing) { fs::remove_all(setAside, ec); }

    ArchiveImportResult out;
    out.imported    = true;
    out.projectRoot = target.string();
    out.projectID   = projectID;
    worlds::WorldStore store{services_};
    for (std::size_t i = 0; i < worldList.size(); ++i) {
        const auto& w = worldList[i];
        if (auto linked = store.relink(out.projectRoot, w.id, packages[i]); !linked.ok()) {
            cleanup();
            return R::failure(linked.error());
        }
        out.worlds.push_back({w.id, packages[i], w.existing.empty()});
    }
    cleanup();
    return R::success(std::move(out));
}

} // namespace scrivi::project_package
