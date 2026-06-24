#include "util/MarkdownStrip.hpp"

#include <cctype>
#include <string>
#include <string_view>

namespace scrivi::util {

namespace {

bool isHorizontalRule(std::string_view line) {
    // A line of only -, *, or _ (>= 3, ignoring spaces) is a thematic break.
    char marker = 0;
    int  count  = 0;
    for (char c : line) {
        if (c == ' ' || c == '\t') { continue; }
        if (c != '-' && c != '*' && c != '_') { return false; }
        if (marker == 0) { marker = c; }
        else if (c != marker) { return false; }
        ++count;
    }
    return count >= 3;
}

// Drops leading blockquote (>) and list markers (-, *, +, 1.) from a line,
// returning the remaining content with the leading whitespace consumed.
std::string_view stripLinePrefixes(std::string_view line) {
    std::size_t i = 0;
    bool changed  = true;
    while (changed) {
        changed = false;
        // leading whitespace
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) { ++i; }
        if (i >= line.size()) { break; }

        // blockquote marker
        if (line[i] == '>') {
            ++i;
            changed = true;
            continue;
        }
        // bullet list marker: -, *, + followed by a space
        if ((line[i] == '-' || line[i] == '*' || line[i] == '+')
            && i + 1 < line.size() && line[i + 1] == ' ') {
            i += 2;
            changed = true;
            continue;
        }
        // ordered list marker: digits followed by '.' or ')' then a space
        std::size_t j = i;
        while (j < line.size() && std::isdigit(static_cast<unsigned char>(line[j]))) { ++j; }
        if (j > i && j < line.size() && (line[j] == '.' || line[j] == ')')
            && j + 1 < line.size() && line[j + 1] == ' ') {
            i = j + 2;
            changed = true;
            continue;
        }
    }
    return line.substr(i);
}

// Removes inline markup from a single line's content (markers only — text kept).
std::string stripInline(std::string_view line) {
    std::string out;
    out.reserve(line.size());

    for (std::size_t i = 0; i < line.size();) {
        char c = line[i];

        // Inline code: copy the span between backticks verbatim, drop the ticks.
        if (c == '`') {
            std::size_t j = i + 1;
            while (j < line.size() && line[j] != '`') { out.push_back(line[j]); ++j; }
            i = (j < line.size()) ? j + 1 : j;
            continue;
        }

        // Image: ![alt](url) -> alt. Link: [text](url) -> text.
        if (c == '!' && i + 1 < line.size() && line[i + 1] == '[') {
            ++i; // skip '!', fall through to link handling on '['
            c = line[i];
        }
        if (c == '[') {
            std::size_t close = line.find(']', i + 1);
            if (close != std::string_view::npos) {
                std::string inner = stripInline(line.substr(i + 1, close - (i + 1)));
                out += inner;
                std::size_t after = close + 1;
                // Drop an immediately following (url) or [ref] target.
                if (after < line.size() && line[after] == '(') {
                    std::size_t paren = line.find(')', after + 1);
                    if (paren != std::string_view::npos) { after = paren + 1; }
                } else if (after < line.size() && line[after] == '[') {
                    std::size_t bracket = line.find(']', after + 1);
                    if (bracket != std::string_view::npos) { after = bracket + 1; }
                }
                i = after;
                continue;
            }
        }

        // Emphasis / strong / strikethrough markers: drop, keep text.
        if (c == '*' || c == '_' || c == '~') { ++i; continue; }

        // Escaped character: keep the escaped char, drop the backslash.
        if (c == '\\' && i + 1 < line.size()) { out.push_back(line[i + 1]); i += 2; continue; }

        out.push_back(c);
        ++i;
    }
    return out;
}

void appendTrimmedRight(std::string& s, std::string_view piece) {
    std::size_t end = piece.size();
    while (end > 0 && (piece[end - 1] == ' ' || piece[end - 1] == '\t')) { --end; }
    s += piece.substr(0, end);
}

} // namespace

std::string stripMarkdown(std::string_view markdown) {
    std::string result;
    result.reserve(markdown.size());

    bool inFence       = false;  // inside ``` or ~~~ fenced block
    bool lastWasBlank  = true;   // suppress leading/consecutive blank lines

    std::size_t pos = 0;
    while (pos <= markdown.size()) {
        std::size_t nl = markdown.find('\n', pos);
        std::string_view line = markdown.substr(
            pos, (nl == std::string_view::npos ? markdown.size() : nl) - pos);
        // Drop a trailing CR for CRLF input.
        if (!line.empty() && line.back() == '\r') { line.remove_suffix(1); }

        // Fence toggling — keep code text, drop the fence lines themselves.
        std::string_view trimmedFront = line;
        while (!trimmedFront.empty()
               && (trimmedFront.front() == ' ' || trimmedFront.front() == '\t')) {
            trimmedFront.remove_prefix(1);
        }
        if (trimmedFront.substr(0, 3) == "```" || trimmedFront.substr(0, 3) == "~~~") {
            inFence = !inFence;
            if (nl == std::string_view::npos) { break; }
            pos = nl + 1;
            continue;
        }

        std::string plain;
        if (inFence) {
            plain = std::string(line); // verbatim code text
        } else if (isHorizontalRule(line)) {
            plain.clear(); // becomes a blank line
        } else {
            std::string_view body = stripLinePrefixes(line);
            // Strip leading ATX heading hashes.
            std::size_t h = 0;
            while (h < body.size() && body[h] == '#') { ++h; }
            if (h > 0 && h <= 6 && (h == body.size() || body[h] == ' ')) {
                body = body.substr(h);
                while (!body.empty() && body.front() == ' ') { body.remove_prefix(1); }
            }
            plain = stripInline(body);
        }

        bool blank = true;
        for (char c : plain) {
            if (c != ' ' && c != '\t') { blank = false; break; }
        }

        if (blank) {
            if (!lastWasBlank) { result += '\n'; }
            lastWasBlank = true;
        } else {
            if (!lastWasBlank) { result += '\n'; }
            appendTrimmedRight(result, plain);
            lastWasBlank = false;
        }

        if (nl == std::string_view::npos) { break; }
        pos = nl + 1;
    }

    // Trim a trailing newline left by a final blank line.
    while (!result.empty() && (result.back() == '\n' || result.back() == ' ')) {
        result.pop_back();
    }
    return result;
}

} // namespace scrivi::util
