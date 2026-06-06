#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class ChapterReorderer {
public:
    explicit ChapterReorderer(CoreServices& services);

    [[nodiscard]] Result<ReorderChapterResult> reorder(const ReorderChapterRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
