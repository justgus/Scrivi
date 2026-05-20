#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <vector>

namespace scrivi::manuscript {

struct ResolvedScene {
    SceneID      sceneID;
    ChapterID    chapterID;
    std::string  title;
    Slug         slug;
    std::string  status;
    RelativePath metadataPath;
    RelativePath contentPath;
};

// Walks the on-disk project structure (manuscript.meta.json → chapter.meta.json
// → scene.meta.json) and returns scenes in manuscript order.
class ManuscriptOrderResolver {
public:
    explicit ManuscriptOrderResolver(CoreServices& services);

    Result<std::vector<ResolvedScene>> resolve(const AbsolutePath& projectRoot);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
