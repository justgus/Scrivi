#include "git/SystemGitProvider.hpp"

#include "scrivi/Error.hpp"
#include "util/Process.hpp"

#include <sstream>

namespace scrivi::git {

bool SystemGitProvider::available() {
    return util::executableInPath("git");
}

static Result<void> requireGit() {
    if (!SystemGitProvider::available()) {
        return Result<void>::failure(
            {.code=ErrorCode::gitUnavailable, .message="git is not available in PATH"});
    }
    return Result<void>::success();
}

Result<bool> SystemGitProvider::isRepository(const AbsolutePath& rootPath) {
    auto check = requireGit();
    if (!check.ok()) { return Result<bool>::failure(check.error());
}

    auto r = util::runProcess("git", {"rev-parse", "--is-inside-work-tree"}, rootPath);
    if (!r.ok()) { return Result<bool>::failure(r.error());
}
    return Result<bool>::success(r.value().exitCode == 0);
}

Result<void> SystemGitProvider::initRepository(const AbsolutePath& rootPath) {
    auto check = requireGit();
    if (!check.ok()) { return check;
}

    auto r = util::runProcess("git", {"init"}, rootPath);
    if (!r.ok()) { return Result<void>::failure(r.error());
}
    if (r.value().exitCode != 0) {
        return Result<void>::failure(
            {.code=ErrorCode::gitError, .message="git init failed: " + r.value().stdout_});
    }
    return Result<void>::success();
}

Result<void> SystemGitProvider::addAll(const AbsolutePath& rootPath) {
    auto check = requireGit();
    if (!check.ok()) { return check;
}

    auto r = util::runProcess("git", {"add", "-A"}, rootPath);
    if (!r.ok()) { return Result<void>::failure(r.error());
}
    if (r.value().exitCode != 0) {
        return Result<void>::failure(
            {.code=ErrorCode::gitError, .message="git add failed: " + r.value().stdout_});
    }
    return Result<void>::success();
}

Result<CommitID> SystemGitProvider::commit(
    const AbsolutePath& rootPath,
    const CommitRequest& req)
{
    auto check = requireGit();
    if (!check.ok()) { return Result<CommitID>::failure(check.error());
}

    auto authorStr      = req.author.name + " <" + req.author.email + ">";
    auto configName     = "user.name=" + req.author.name;
    auto configEmail    = "user.email=" + req.author.email;

    auto r = util::runProcess("git",
        {"-c", configName,
         "-c", configEmail,
         "commit",
         "--allow-empty",
         "--author", authorStr,
         "-m", req.message},
        rootPath);

    if (!r.ok()) { return Result<CommitID>::failure(r.error());
}
    if (r.value().exitCode != 0) {
        return Result<CommitID>::failure(
            {.code=ErrorCode::gitError, .message="git commit failed: " + r.value().stdout_});
    }

    // Read the commit hash
    auto hashR = util::runProcess("git", {"rev-parse", "HEAD"}, rootPath);
    if (!hashR.ok()) { return Result<CommitID>::failure(hashR.error());
}

    std::string hash = hashR.value().stdout_;
    // Trim trailing whitespace/newline
    while (!hash.empty() && (hash.back() == '\n' || hash.back() == '\r' || hash.back() == ' ')) {
        hash.pop_back();
}

    return Result<CommitID>::success(CommitID{hash});
}

Result<GitStatus> SystemGitProvider::status(const AbsolutePath& rootPath) {
    auto check = requireGit();
    if (!check.ok()) { return Result<GitStatus>::failure(check.error());
}

    auto isRepoR = isRepository(rootPath);
    if (!isRepoR.ok()) { return Result<GitStatus>::failure(isRepoR.error());
}

    GitStatus gs;
    gs.isRepository = isRepoR.value();

    if (!gs.isRepository) {
        return Result<GitStatus>::success(std::move(gs));
    }

    // Porcelain output: each modified file on its own line with a 2-char status code
    auto r = util::runProcess("git", {"status", "--porcelain"}, rootPath);
    if (!r.ok()) { return Result<GitStatus>::failure(r.error());
}
    if (r.value().exitCode != 0) {
        return Result<GitStatus>::failure(
            {.code=ErrorCode::gitError, .message="git status failed: " + r.value().stdout_});
    }

    std::istringstream ss(r.value().stdout_);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.size() < 3) { continue;
}
        char xy   = line[0]; // index status
        char xy2  = line[1]; // worktree status
        std::string path = line.substr(3);

        if (xy == '?' && xy2 == '?') {
            gs.untrackedFiles.push_back(path);
        } else {
            gs.changedFiles.push_back(path);
        }
    }

    gs.hasUncommittedChanges = !gs.changedFiles.empty() || !gs.untrackedFiles.empty();
    return Result<GitStatus>::success(std::move(gs));
}

} // namespace scrivi::git
