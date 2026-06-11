#pragma once

#include "scrivi/Result.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace scrivi::schemas {

struct BandLayout {
    std::string bandID;
    std::string label;
    std::string color;       // hex, e.g. "#5B8DD9"
    double      proportion = 0.0;  // 0 < proportion ≤ 1.0; sum over all bands = 1.0
};

struct StoryStructureData {
    std::string              activeStructureID;
    std::vector<BandLayout>  bandLayout;
    std::vector<BandLayout>  customBands;   // non-empty only when activeStructureID == "custom"
};

std::string serializeStoryStructure(const StoryStructureData& data);
Result<StoryStructureData> parseStoryStructure(std::string_view json);

} // namespace scrivi::schemas
