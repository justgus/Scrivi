#pragma once
// SP-174 / T-0601 — the escape layer's PRESENTATION rules (EP-045 E1), in the core. A port of Apple's `ManuscriptEscapes.swift`
// (as Linux's `ManuscriptEscapes.cpp` already was), in UTF-16 units. One of the 32 ASCII marks behind a backslash shows as the
// mark alone; a backslash before a newline that a non-blank line follows is a HARD LINE BREAK, also hidden.

#include "markdown/MarkdownText.hpp"

#include <vector>

namespace scrivi::markdown::escapes {

[[nodiscard]] bool isEscapable(char16_t c);

struct Map {
    std::vector<int> presentedToSource;   // an escaped mark maps to BEFORE its backslash
    std::vector<int> sourceToPresented;   // the boundary between `\` and its mark → before the mark
    Units presented;
};

// `continues`: the text continues past `source` with a non-blank line (a line-end backslash is then a hard break).
[[nodiscard]] Map map(UnitsView source, bool continues = false);

} // namespace scrivi::markdown::escapes
