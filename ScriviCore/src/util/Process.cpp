#include "util/Process.hpp"

#include "scrivi/Error.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#ifdef _WIN32
  #include <io.h>
  #define popen  _popen
  #define pclose _pclose
#else
  #include <sys/wait.h>
  #include <unistd.h>
  #ifdef __APPLE__
    #include <TargetConditionals.h>  // Apple-only; defines TARGET_OS_IPHONE (used below)
  #endif
#endif

// iOS/iPadOS/visionOS (and the other embedded Apple platforms) sandbox out
// subprocess spawning: std::system / popen are marked unavailable in their SDKs.
// Scrivi never shells out on those platforms — git snapshots are macOS/desktop
// only, and SystemGitProvider::available() already treats "no git" as a graceful
// no-op. So compile the subprocess path out there and report git/tools as absent.
#if defined(__APPLE__) && TARGET_OS_IPHONE
  #define SCRIVI_NO_SUBPROCESS 1
#else
  #define SCRIVI_NO_SUBPROCESS 0
#endif

namespace scrivi::util {

bool executableInPath(const std::string& name) {
#if SCRIVI_NO_SUBPROCESS
    (void)name;
    return false; // no subprocess support on this platform
#elif defined(_WIN32)
    // On Windows, use where.exe
    std::string cmd = "where " + name + " >NUL 2>&1";
    return std::system(cmd.c_str()) == 0; // NOLINT(bugprone-command-processor)
#else
    std::string cmd = "command -v " + name + " >/dev/null 2>&1";
    return std::system(cmd.c_str()) == 0; // NOLINT(bugprone-command-processor)
#endif
}

Result<ProcessResult> runProcess(
    const std::string&              executable,
    const std::vector<std::string>& args,
    const std::string&              workingDirectory)
{
#if SCRIVI_NO_SUBPROCESS
    (void)executable;
    (void)args;
    (void)workingDirectory;
    return Result<ProcessResult>::failure(
        {.code    = ErrorCode::internalError,
         .message = "Subprocess execution is not available on this platform"});
#else
    // Build the shell command: cd <workingDirectory> && <executable> <args...> 2>&1
    std::ostringstream cmd;
    cmd << "cd ";

    // Quote the working directory to handle spaces
    cmd << "\"" << workingDirectory << "\"";
    cmd << " && ";
    cmd << "\"" << executable << "\"";
    for (const auto& arg : args) {
        cmd << " \"" << arg << "\"";
    }
    cmd << " 2>&1";

    std::string cmdStr = cmd.str();

    FILE* pipe = popen(cmdStr.c_str(), "r"); // NOLINT(bugprone-command-processor)
    if (pipe == nullptr) {
        return Result<ProcessResult>::failure(
            {.code = ErrorCode::internalError, .message = "Failed to launch process: " + cmdStr});
    }

    std::string output;
    std::array<char, 256> buffer;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output += buffer.data();
    }

    int exitCode = pclose(pipe);
#ifndef _WIN32
    // WEXITSTATUS extracts the actual exit status on POSIX
    if (WIFEXITED(exitCode)) {
        exitCode = WEXITSTATUS(exitCode);
    }
#endif

    ProcessResult result;
    result.exitCode = exitCode;
    result.stdout_  = output;
    return Result<ProcessResult>::success(std::move(result));
#endif // SCRIVI_NO_SUBPROCESS
}

} // namespace scrivi::util
