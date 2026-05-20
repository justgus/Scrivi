#pragma once

#include "scrivi/Result.hpp"

#include <string>
#include <vector>

namespace scrivi::util {

struct ProcessResult {
    int         exitCode = 0;
    std::string stdout_;
    std::string stderr_;
};

// Runs a command with arguments in the given working directory.
// Returns failure if the process cannot be launched.
// A non-zero exit code is returned in ProcessResult.exitCode, not as a failure.
Result<ProcessResult> runProcess(
    const std::string&              executable,
    const std::vector<std::string>& args,
    const std::string&              workingDirectory);

// Returns true if the given executable can be found in PATH.
bool executableInPath(const std::string& name);

} // namespace scrivi::util
