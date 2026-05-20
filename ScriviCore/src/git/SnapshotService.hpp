#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::git {

class SnapshotService {
public:
    explicit SnapshotService(CoreServices& services);

    Result<EnableGitResult>      enable(const EnableGitRequest& request);
    Result<CreateSnapshotResult> createSnapshot(const CreateSnapshotRequest& request);

private:
    CoreServices& services_;

    Result<void> writeGitignore(const AbsolutePath& projectRoot);
    Result<SnapshotID> commitSnapshot(
        const AbsolutePath& projectRoot,
        const AuthorshipRef& author,
        const std::string& label,
        const std::string& snapshotID,
        const std::string& timestamp);
    Result<void> appendSnapshotMetadata(
        const AbsolutePath& projectRoot,
        const std::string& snapshotID,
        const CommitID& commitID,
        const std::string& label,
        const std::string& note,
        const std::string& timestamp,
        const AuthorshipRef& author);
};

} // namespace scrivi::git
