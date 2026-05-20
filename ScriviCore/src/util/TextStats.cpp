#include "TextStats.hpp"

#include <cctype>

namespace scrivi::util {

TextStatsResult countText(std::string_view utf8Markdown) {
    TextStatsResult stats;

    bool inWord = false;

    for (unsigned char ch : utf8Markdown) {
        // Count every byte that starts a UTF-8 code point (not a continuation byte)
        if ((ch & 0xC0) != 0x80)
            ++stats.characterCount;

        // Word detection: split on ASCII whitespace; non-ASCII bytes are word chars
        bool isSpace = (ch <= 127 && std::isspace(ch));
        if (!isSpace) {
            if (!inWord) {
                ++stats.wordCount;
                inWord = true;
            }
        } else {
            inWord = false;
        }
    }

    return stats;
}

} // namespace scrivi::util
