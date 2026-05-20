#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::git {

// Invokes the system `git` binary via the Process utility.
// All operations fail with gitError if git is not found in PATH.
class SystemGitProvider final : public GitProvider {
public:
    Result<bool>     isRepository(const AbsolutePath& rootPath)                       override;
    Result<void>     initRepository(const AbsolutePath& rootPath)                     override;
    Result<void>     addAll(const AbsolutePath& rootPath)                             override;
    Result<CommitID> commit(const AbsolutePath& rootPath, const CommitRequest& req)   override;
    Result<GitStatus> status(const AbsolutePath& rootPath)                            override;

    // Returns true if `git` is available in PATH.
    static bool available();
};

} // namespace scrivi::git
