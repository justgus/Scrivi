#include "platform/AppSupportLayout.hpp"

#include "util/PathUtils.hpp"

namespace scrivi::util {

Result<void> bootstrapAppSupport(const AbsolutePath& appSupportRoot,
                                 FileSystem& fs) {
    static const char* const kSubdirs[] = {
        "identity",
        "state/projects",
        "cache/projects",
        "logs",
        "tmp",
    };

    for (const auto* sub : kSubdirs) {
        auto path = join(appSupportRoot, sub);
        auto result = fs.createDirectories(path);
        if (!result.ok()) return result;
    }

    return Result<void>::success();
}

} // namespace scrivi::util
