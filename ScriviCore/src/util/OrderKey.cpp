#include "util/OrderKey.hpp"

#include <algorithm>

namespace scrivi::util {

namespace {

// Base-62 alphabet in ASCII order: '0'<..<'9'<'A'<..<'Z'<'a'<..<'z'. Its byte order is
// its digit order, so std::string comparison equals key comparison.
constexpr std::string_view kDigits =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
constexpr int kBase = 62;

// Digit value of a character, or -1 if not in the alphabet.
int digitVal(char c) {
    const auto pos = kDigits.find(c);
    return pos == std::string_view::npos ? -1 : static_cast<int>(pos);
}

char digitChar(int v) { return kDigits[static_cast<std::size_t>(v)]; }

// The i-th digit value of key `k`, treating positions past the end as 0 (the implicit
// low pad — a shorter key is the prefix of infinitely many "…000" keys). '0' == value 0.
int digitAt(std::string_view k, std::size_t i) {
    return i < k.size() ? digitVal(k[i]) : 0;
}

} // namespace

std::string keyBetween(std::string_view lo, std::string_view hi) {
    // hi == "" means "+infinity" (after everything); lo == "" means "-infinity".
    const bool hiOpen = hi.empty();
    const bool loOpen = lo.empty();

    // Misuse guard: a bounded range must be ordered lo < hi.
    if (!loOpen && !hiOpen && lo >= hi) {
        return {};
    }

    std::string out;
    // Walk digit positions, building the smallest key strictly greater than lo and
    // strictly less than hi. At each position we have a lo-digit and a hi-digit (hi
    // treated as base when open, so we can always go "up" toward it).
    for (std::size_t i = 0;; ++i) {
        const int loDigit = digitAt(lo, i);
        // When hi is open there is no upper digit — the ceiling is the base (kBase), so
        // any digit > loDigit works. When hi is bounded, past its end it reads as 0.
        const int hiDigit = hiOpen ? kBase : digitAt(hi, i);

        // Is there room for a strictly-in-between digit at this position?
        const int mid = (loDigit + hiDigit) / 2;
        if (mid > loDigit) {
            // Pick the midpoint digit; everything to the left equals lo's prefix, and
            // mid > loDigit guarantees out > lo, while mid < hiDigit (since mid is a
            // floor and mid>loDigit≥... ) guarantees out < hi.
            out += digitChar(mid);
            return out;
        }

        // No gap here (hiDigit == loDigit, or they're adjacent with no integer between):
        // copy lo's digit and descend to the next position. We must keep out > lo
        // eventually, which happens once hi's constraint loosens (open) or lo runs out.
        out += digitChar(loDigit);

        // Safety: if hi is bounded and we've consumed all of hi's digits, the remaining
        // room is below hi — append a digit above lo's next digit. Handled naturally by
        // the next iteration because past hi's end hiDigit reads 0, which can't be > any
        // loDigit; but past LO's end loDigit reads 0 and hiOpen/next hi digit gives room.
        // Guard against pathological non-termination with a generous cap.
        if (i > lo.size() + hi.size() + 8) {
            // Should never happen for valid inputs; return a key one step above lo.
            out += digitChar(1);
            return out;
        }
    }
}

std::string keyBefore(std::string_view hi) { return keyBetween(std::string_view{}, hi); }
std::string keyAfter(std::string_view lo)  { return keyBetween(lo, std::string_view{}); }

bool isOrderKey(std::string_view s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (digitVal(c) < 0) return false;
    }
    // A key ending in the lowest digit ('0') would have no room to insert before the
    // next key at the same length and is not canonical; treat it as ill-formed.
    return s.back() != '0';
}

} // namespace scrivi::util
