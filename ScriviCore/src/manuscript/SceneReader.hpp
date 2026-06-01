#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

namespace scrivi::manuscript {

class SceneReader {
public:
    explicit SceneReader(CoreServices& services);

    // Reads Markdown content from contentPath (relative to projectRoot).
    [[nodiscard]] Result<Utf8Text> readContent(const AbsolutePath& projectRoot,
                                 const RelativePath& contentPath) const;

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
