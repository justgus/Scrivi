#pragma once

#include "scrivi/Services.hpp"

#include <string>
#include <vector>

namespace scrivi::mocks {

class MockGitProvider final : public GitProvider {
public:
    // Configuration
    bool shouldFail          = false;
    bool repoExists          = false;
    std::string failMessage  = "mock git failure";

    // Call recording
    std::vector<std::string> initCalls;
    std::vector<std::string> addAllCalls;
    std::vector<CommitRequest> commitRequests;
    std::vector<std::string> statusCalls;

    Result<bool> isRepository(const AbsolutePath& rootPath) override {
        if (shouldFail) return Result<bool>::failure({ErrorCode::gitError, failMessage});
        return Result<bool>::success(repoExists);
    }

    Result<void> initRepository(const AbsolutePath& rootPath) override {
        if (shouldFail) return Result<void>::failure({ErrorCode::gitError, failMessage});
        initCalls.push_back(rootPath);
        repoExists = true;
        return Result<void>::success();
    }

    Result<void> addAll(const AbsolutePath& rootPath) override {
        if (shouldFail) return Result<void>::failure({ErrorCode::gitError, failMessage});
        addAllCalls.push_back(rootPath);
        return Result<void>::success();
    }

    Result<CommitID> commit(const AbsolutePath& rootPath, const CommitRequest& request) override {
        if (shouldFail) return Result<CommitID>::failure({ErrorCode::gitError, failMessage});
        commitRequests.push_back(request);
        CommitID id{"mock-commit-" + std::to_string(commitRequests.size())};
        return Result<CommitID>::success(std::move(id));
    }

    Result<GitStatus> status(const AbsolutePath& rootPath) override {
        if (shouldFail) return Result<GitStatus>::failure({ErrorCode::gitError, failMessage});
        statusCalls.push_back(rootPath);
        return Result<GitStatus>::success(GitStatus{repoExists, false, {}, {}});
    }
};

} // namespace scrivi::mocks
