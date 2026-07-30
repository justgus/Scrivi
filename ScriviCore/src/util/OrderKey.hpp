#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::util {

// OrderKey — lexicographically-sortable fractional ordering keys (EP-027 A4b).
//
// A chapter's on-disk folder is named `chapter-<orderKey>`. Sorting the folders by
// their key (plain std::string/byte order) yields manuscript reading order, so the
// filesystem itself is the source of truth for order (B3). Inserting or moving a
// chapter picks a NEW key strictly between its two neighbours' keys — renaming only
// that one folder, never the others (the whole point of A4b vs. positional renumber).
//
// GENERATION alphabet (T-0358): keys are now generated from base-36 CAPITALS-ONLY
// 0-9 A-Z, whose ASCII order IS the sort order. Capitals-only guarantees identical sort
// on every filesystem (no case-fold ambiguity) and keeps keys short and legible.
//
// A key never ends in the lowest digit ('0') so there is always room to insert before it.
//
// keyBetween(lo, hi) returns a key K with lo < K < hi (lexicographically). An empty lo
// means "before everything"; an empty hi means "after everything". If bounded bounds are
// passed out of order (lo > hi) they are auto-repaired by swapping and a valid key is still
// returned. Only EQUAL bounded bounds (lo == hi, neither open) yield an empty string.
//
// Occasional key growth is expected (repeated inserts between two adjacent keys lengthen
// the key); a global rebalance (rebalancedKeys) reassigns short evenly-spaced caps keys —
// a manual maintenance op (the scrivi_rebalance_* repair endpoint), not the insert path.
std::string keyBetween(std::string_view lo, std::string_view hi);

// Convenience: a key before `hi` (== keyBetween("0", hi)) and after `lo`
// (== keyBetween(lo, "Z")).
std::string keyBefore(std::string_view hi);
std::string keyAfter(std::string_view lo);

// Rebalance (T-0358): return `n` short, strictly-ascending, evenly-spread caps-only keys
// (0-9 A-Z) for reassigning to `n` items in reading order. Single-char keys are used while
// n fits the spread (drawn from positions 1..35 so there is room before the first and
// after the last); beyond that the spread falls back to two-char keys. Never returns '0'
// or a key ending in '0'. Empty vector for n == 0.
std::vector<std::string> rebalancedKeys(std::size_t n);

// Is `s` a well-formed order key? Non-empty, only alphabet digits (the permissive
// 0-9 A-Z a-z range), not ending in '0'. Used to detect old-format (non-order-key) folder
// names during migration — deliberately accepts legacy lowercase keys (T-0358).
bool isOrderKey(std::string_view s);

} // namespace scrivi::util
