#pragma once

#include <string>
#include <string_view>

namespace scrivi::util {

// Converts Markdown source to plain text suitable for full-text indexing.
//
// This is a lightweight, lossy strip — not a CommonMark parser. It removes the
// common inline and block markup a Scrivi manuscript contains so that Spotlight
// indexes the words an author wrote, not the syntax around them:
//   - ATX headings (`#`..`######`) lose their leading hashes
//   - emphasis/strong/strikethrough markers (* _ ~) are removed, text kept
//   - inline code (`code`) and fenced/indented code blocks keep their text
//   - links/images keep the visible text, drop the URL/target
//   - blockquote (`>`) and list markers (-, *, +, 1.) are removed
//   - horizontal rules become blank lines
// The result is whitespace-normalised: runs of blank lines collapse, trailing
// spaces are trimmed. Both Layer 1 (in-app) and Layer 2 (importer) call this so
// they index identical text. (EP-017 / T-0178, T-0183.)
[[nodiscard]] std::string stripMarkdown(std::string_view markdown);

} // namespace scrivi::util
