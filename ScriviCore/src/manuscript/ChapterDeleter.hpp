#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class ChapterDeleter {
public:
    explicit ChapterDeleter(CoreServices& services);

    [[nodiscard]] Result<DeleteChapterResult> remove(const DeleteChapterRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
