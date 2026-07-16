#pragma once

#include <string>
#include <string_view>

namespace scrivi::util {

// OrderKey — lexicographically-sortable fractional ordering keys (EP-027 A4b).
//
// A chapter's on-disk folder is named `chapter-<orderKey>`. Sorting the folders by
// their key (plain std::string/byte order) yields manuscript reading order, so the
// filesystem itself is the source of truth for order (B3). Inserting or moving a
// chapter picks a NEW key strictly between its two neighbours' keys — renaming only
// that one folder, never the others (the whole point of A4b vs. positional renumber).
//
// Keys use the base-62 alphabet 0-9 A-Z a-z, whose ASCII order IS the sort order, so
// string comparison == key comparison. Keys never contain the terminator; a key never
// ends in the lowest digit ('0') so there is always room to insert before the next one.
//
// keyBetween(lo, hi) returns a key K with lo < K < hi (lexicographically). An empty lo
// means "before everything"; an empty hi means "after everything". Precondition: if both
// are non-empty, lo < hi. On misuse (lo >= hi) it returns an empty string.
//
// Occasional key growth is expected (repeated inserts between two adjacent keys lengthen
// the key); a global rebalance (reassign short evenly-spaced keys) is a rare maintenance
// op, not part of the normal insert path.
std::string keyBetween(std::string_view lo, std::string_view hi);

// Convenience: a key before `hi` (== keyBetween("", hi)) and after `lo`
// (== keyBetween(lo, "")).
std::string keyBefore(std::string_view hi);
std::string keyAfter(std::string_view lo);

// Is `s` a well-formed order key? Non-empty, only alphabet digits, not ending in '0'.
// Used to detect old-format (non-order-key) folder names during migration.
bool isOrderKey(std::string_view s);

} // namespace scrivi::util
