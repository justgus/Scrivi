#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class ChapterRenamer {
public:
    explicit ChapterRenamer(CoreServices& services);

    [[nodiscard]] Result<RenameChapterResult> rename(const RenameChapterRequest& request);

    // Walks the manuscript index to resolve a chapterID to its metadataPath.
    // Available for future operations that need path resolution from an ID.
    [[nodiscard]] Result<RelativePath> findChapterMetadataPath(
        const AbsolutePath& projectRootPath, const ChapterID& chapterID);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
