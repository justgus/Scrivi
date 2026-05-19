#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
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
    virtual SnapshotID   newSnapshotID()   = 0;
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
    virtual Result<std::vector<AbsolutePath>> listDirectory(
        const AbsolutePath& path)                                = 0;
    virtual Result<void> removeFile(const AbsolutePath& path)    = 0;
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

enum class LogLevel { debug, info, warning, error };

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
