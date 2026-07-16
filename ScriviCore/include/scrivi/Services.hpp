#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace scrivi {

using SecretBytes = std::vector<std::byte>;

class Clock {
public:
    virtual ~Clock() = default;
    virtual ISO8601Timestamp nowUTC() = 0;
};

class UUIDProvider {
public:
    virtual ~UUIDProvider() = default;
    virtual ProjectID    newProjectID()    = 0;
    virtual ManuscriptID newManuscriptID() = 0;
    virtual ChapterID    newChapterID()    = 0;
    virtual SceneID      newSceneID()      = 0;
    virtual IdentityID   newIdentityID()   = 0;
    virtual PersonaID    newPersonaID()    = 0;
    virtual SnapshotID        newSnapshotID()        = 0;
    virtual ObjectID          newObjectID()          = 0;
    virtual TimelineID        newTimelineID()        = 0;
    virtual HistoricalEventID newHistoricalEventID() = 0;
};

class FileSystem {
public:
    virtual ~FileSystem() = default;
    virtual Result<bool> exists(const AbsolutePath& path)        = 0;
    virtual Result<bool> isDirectory(const AbsolutePath& path)   = 0;
    virtual Result<void> createDirectories(const AbsolutePath& path) = 0;
    virtual Result<Utf8Text> readTextFile(const AbsolutePath& path) = 0;
    virtual Result<void> atomicWriteTextFile(
        const AbsolutePath& path, std::string_view utf8Text)     = 0;
    // Appends utf8Text to path (creating it if absent), flushing before return.
    // Used by the append-only history log (EP-019 §6.a). Not atomic across a
    // crash mid-append — a torn final line is detected and truncated at load.
    virtual Result<void> appendTextFile(
        const AbsolutePath& path, std::string_view utf8Text)     = 0;
    virtual Result<std::vector<AbsolutePath>> listDirectory(
        const AbsolutePath& path)                                = 0;
    virtual Result<void> removeFile(const AbsolutePath& path)    = 0;
    // Renames/moves a path (file or directory) from `from` to `to`. The move is
    // ATOMIC within a filesystem (the OS `rename` either fully succeeds or fully
    // fails — a crash mid-rename never leaves a half-renamed directory), which is
    // why chapter/scene folder reslugging (EP-027) relies on it. It REFUSES to
    // overwrite: if `to` already exists the call fails (invalidArgument) rather than
    // clobbering — a new destination must never destroy existing content (the class
    // of bug behind I-0072). `from` must exist. A cross-filesystem move (rename's
    // `cross_device_link`) is reported as an ioError rather than silently doing a
    // non-atomic copy+delete; in-package moves are always same-filesystem so this
    // does not arise for chapter/scene folders.
    virtual Result<void> renamePath(
        const AbsolutePath& from, const AbsolutePath& to)        = 0;
};

class SecureStore {
public:
    virtual ~SecureStore() = default;
    virtual Result<bool>        containsSecret(std::string_view key)                      = 0;
    virtual Result<void>        putSecret(std::string_view key, const SecretBytes& value) = 0;
    virtual Result<SecretBytes> getSecret(std::string_view key)                           = 0;
};

struct GitAuthor {
    std::string name;
    std::string email;
};

struct CommitRequest {
    std::string message;
    GitAuthor   author;
};

struct GitStatus {
    bool isRepository            = false;
    bool hasUncommittedChanges   = false;
    std::vector<RelativePath> changedFiles;
    std::vector<RelativePath> untrackedFiles;
};

class GitProvider {
public:
    virtual ~GitProvider() = default;
    virtual Result<bool> isRepository(const AbsolutePath& rootPath)  = 0;
    virtual Result<void> initRepository(const AbsolutePath& rootPath) = 0;
    virtual Result<void> addAll(const AbsolutePath& rootPath)         = 0;
    virtual Result<CommitID> commit(
        const AbsolutePath& rootPath, const CommitRequest& request)  = 0;
    virtual Result<GitStatus> status(const AbsolutePath& rootPath)   = 0;
};

enum class LogLevel : std::uint8_t { debug, info, warning, error };

class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(LogLevel level, std::string_view message) = 0;
};

struct CoreServices {
    FileSystem*   fileSystem   = nullptr;
    SecureStore*  secureStore  = nullptr;
    Clock*        clock        = nullptr;
    UUIDProvider* uuidProvider = nullptr;
    GitProvider*  gitProvider  = nullptr;
    Logger*       logger       = nullptr;
};

} // namespace scrivi
