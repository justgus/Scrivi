#include "domain/Slug.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace scrivi::util {

Slug makeSlug(std::string_view text) {
    std::string result;
    result.reserve(text.size());

    bool lastWasHyphen = true; // suppress leading hyphen

    for (unsigned char ch : text) {
        if (ch > 127) continue; // drop non-ASCII

        if (std::isalnum(ch)) {
            result += static_cast<char>(std::tolower(ch));
            lastWasHyphen = false;
        } else {
            if (!lastWasHyphen && !result.empty()) {
                result += '-';
                lastWasHyphen = true;
            }
        }
    }

    // strip trailing hyphen
    if (!result.empty() && result.back() == '-')
        result.pop_back();

    return result;
}

} // namespace scrivi::util
