#pragma once

#include <cstddef>
#include <string_view>

namespace scrivi::util {

struct TextStatsResult {
    std::size_t wordCount      = 0;
    std::size_t characterCount = 0; // Unicode code points, excluding Markdown syntax
};

// Counts words and characters in a UTF-8 Markdown string.
// Markdown heading/emphasis/link syntax characters are included in character
// count but Markdown fenced code blocks and ATX heading markers are not
// specially stripped — this is a lightweight prose counter, not a renderer.
TextStatsResult countText(std::string_view utf8Markdown);

} // namespace scrivi::util
