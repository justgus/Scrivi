#include "platform/AppSupportLayout.hpp"

#include "scrivi/Error.hpp"
#include "util/PathUtils.hpp"

#include <array>

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <shlobj.h>
  #include <combaseapi.h>
#elifdef __APPLE__
  #include <pwd.h>
  #include <unistd.h>
#else
  // Linux / other POSIX
  #include <cstdlib>
  #include <pwd.h>
  #include <unistd.h>
#endif

#include <filesystem>
#include <string>

namespace scrivi::util {

Result<AbsolutePath> platformDefault() {
#ifdef _WIN32
    PWSTR wpath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData,
                                      KF_FLAG_CREATE, nullptr, &wpath);
    if (FAILED(hr)) {
        return Result<AbsolutePath>::failure(
            Error{ErrorCode::ioError, "SHGetKnownFolderPath failed"});
    }
    std::filesystem::path base{wpath};
    CoTaskMemFree(wpath);
    return Result<AbsolutePath>::success((base / "Scrivi").string());

#elifdef __APPLE__
    // On Apple platforms the caller (AppEnvironment.swift / ScriviCoreAdapter)
    // supplies the path from FileManager.applicationSupportDirectory, which is
    // the correct sandboxed location. platformDefault() here covers headless /
    // non-sandboxed C++ usage (ctest, CLI tools).
    const char* home = std::getenv("HOME");
    if ((home == nullptr) || home[0] == '\0') {
        struct passwd* pw = getpwuid(getuid());
        if (pw == nullptr) { return Result<AbsolutePath>::failure(
                Error{.code=ErrorCode::ioError, .message="Cannot determine HOME"});
}
        home = pw->pw_dir;
    }
    std::filesystem::path base{home};
    base /= "Library/Application Support/Scrivi";
    return Result<AbsolutePath>::success(base.string());

#else
    // Linux / other POSIX: prefer $XDG_DATA_HOME, fall back to ~/.local/share
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (xdg && xdg[0] != '\0') {
        return Result<AbsolutePath>::success(
            (std::filesystem::path{xdg} / "Scrivi").string());
    }
    const char* home = std::getenv("HOME");
    if (!home || home[0] == '\0') {
        struct passwd* pw = getpwuid(getuid());
        if (!pw) return Result<AbsolutePath>::failure(
                Error{ErrorCode::ioError, "Cannot determine HOME"});
        home = pw->pw_dir;
    }
    std::filesystem::path base{home};
    base /= ".local/share/Scrivi";
    return Result<AbsolutePath>::success(base.string());
#endif
}

Result<void> bootstrapAppSupport(const AbsolutePath& appSupportRoot,
                                 FileSystem& fs) {
    static constexpr std::array<const char*, 5> kSubdirs = {
        "identity",
        "state/projects",
        "cache/projects",
        "logs",
        "tmp",
    };

    for (const auto* sub : kSubdirs) {
        auto path = join(appSupportRoot, sub);
        auto result = fs.createDirectories(path);
        if (!result.ok()) { return result;
}
    }

    return Result<void>::success();
}

} // namespace scrivi::util
