#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class ChapterCreator {
public:
    explicit ChapterCreator(CoreServices& services);

    [[nodiscard]] Result<CreateChapterResult> create(const CreateChapterRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
