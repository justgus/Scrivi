#include "util/OrderKey.hpp"

#include <algorithm>
#include <vector>

namespace scrivi::util {

namespace {

// GENERATION alphabet (T-0358): base-36 CAPITALS-ONLY, ASCII order '0'<..<'9'<'A'<..<'Z'.
// A key is a sequence of these single digits joined by '.' — e.g. "A", "A.5", "Q.3.T".
// The DOT MATTERS for cross-filesystem sorting: '.' (0x2E) sorts before every key digit,
// and because each dot-segment is exactly ONE digit there is never a multi-character run
// that macOS "natural/numeric" sort could re-order differently from byte sort. So byte
// order (ScriviCore's authority) == Finder/Explorer order on every platform.
constexpr std::string_view kGenDigits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr unsigned int kGenBase = 36;

// Return the sort order position of the character
// if the character given is below the range of kGenDigits, then return 0
// if the character given is above the range of kGenDigits, then return kGenDigits.length() -1
unsigned int genDigitVal(char c) {
    if (c < '0') {
        return 0;
    }
    const auto pos = kGenDigits.find(c);
    return pos == std::string_view::npos ? static_cast<unsigned int>(kGenDigits.length() - 1) : static_cast<unsigned int>(pos);
}

// Return the character represented by the sort order position given.  The input value
// cannot be negative.  if the value is 0 return '0' if the value is 36 or greater (which
// would result in an array index out of bounds), return 'Z'
char genDigitChar(unsigned int v) {
    if (v < kGenBase) {
        return kGenDigits[static_cast<std::size_t>(v)];
    } else {
        return 'Z';
    }
} //end genDigiChar

// Parse a dotted key ("A.5.H") into its digit values ([10,5,17]). A non-digit segment
// (legacy lowercase, or malformed) yields its best-effort rank so comparisons stay sane;
// generation only runs against caps keys post-rebalance, so this is a safety net.
std::vector<unsigned int> toDigits(std::string_view k) {
    std::vector<unsigned int> out;
    for (std::size_t i = 0; i < k.size(); ++i) {
        if (k[i] == '.') continue;              // separator — segments are single digits
        const unsigned int v = genDigitVal(k[i]);
        if (v >= 0 && v < kGenBase) {
            out.push_back(v);
        } else if (k[i] >= 'a' && k[i] <= 'z') {
            out.push_back(kGenBase + (k[i] - 'a')); // legacy rank
        } else {
            out.push_back(kGenBase - 1);
        } //end ifs
    } //end for
    return out;
}

// Render a digit list back to a dotted key: first digit plain, a dot before each rest.
std::string fromDigits(const std::vector<unsigned int>& ds) {
    std::string out;
    for (std::size_t i = 0; i < ds.size(); ++i) {
        if (i > 0) out += '.';
        out += genDigitChar(ds[i]);
    }
    return out;
}

// i-th digit of a parsed list, past the end reads as 0 (the implicit low pad).
int digitAt(const std::vector<unsigned int>& ds, std::size_t i) {
    return i < ds.size() ? ds[i] : 0;
}

} // namespace

std::string keyBetween(std::string_view lo, std::string_view hi) {
    const bool hiOpen = (hi == "") || (hi[0] == kGenDigits[kGenBase-1]);
    const bool loOpen = (lo == "") || (lo[0] == kGenDigits[0]);

    if (hiOpen && loOpen) { //if both are the outer bounds, then return any value as a start
        return "A";
    }

    // Normalize the empty open-bound sentinels to their concrete reserved digits so the
    // ordering guard, boundary checks, and digit parsing below all work on real strings.
    // "" as lo means the open bottom ('0'); "" as hi means the open top ('Z').
    if (lo == "") lo = kGenDigits.substr(0, 1);              // ""  -> "0"
    if (hi == "") hi = kGenDigits.substr(kGenBase - 1, 1);   // ""  -> "Z"

    // Misuse guard: a bounded range must be ordered lo < hi (compare as raw keys).
    // order correctly if possible
    if (lo > hi) { //lo > hi, values are out of order
        if (hiOpen) { //hi == 'Z' and lo > hi
            return {}; //lo is out of range above hi
        } else if (loOpen) { //lo == '0' and hi < lo
            return {}; //hi isout of range below lo
        } else {
            std::string_view t = lo;
            lo = hi;
            hi = t;
        }
    } else if (lo == hi) {
        return {};
    }
    // lo is now < hi.  next check boundaries
    if (lo < "0") {
        return {}; //lo is out of range
    }
    if (hi > "Z") {
        return {}; //hi is out of range
    }
    
    const std::vector<unsigned int> loD = toDigits(lo);
    const std::vector<unsigned int> hiD = toDigits(hi);

    // Bounded fractional descent. We walk digit positions left-to-right, building `out`.
    // `belowHi` becomes true once we have placed a digit strictly below hi's ceiling at
    // some position — after that hi no longer constrains us and only lo's floor matters.
    //
    // The loop is bounded by `maxLen`: we can only need to inspect as many positions as the
    // longer of the two inputs, plus ONE extra position to open a fresh dot-segment when lo
    // and hi are adjacent (hi == lo's prefix + a small digit). No unbounded growth.
    const std::size_t maxLen = std::max(loD.size(), hiD.size()) + 1;

    std::vector<unsigned int> out;
    bool belowHi = false;
    for (std::size_t i = 0; i < maxLen; ++i) {
        const unsigned int loDigit = digitAt(loD, i);   // past lo's end → 0 (implicit floor)
        // Ceiling at this position: if we are already strictly below hi, or hi is the open
        // top, or we have run past hi's digits, the only cap is the alphabet's top real
        // digit (kGenBase-1 == 'Z', reserved, so a produced digit stays <= 'Y'). Otherwise
        // hi's actual digit here is the ceiling.
        const unsigned int hiDigit = (belowHi || hiOpen || i >= hiD.size())
                                         ? (kGenBase - 1)
                                         : digitAt(hiD, i);

        // Try to place a digit strictly greater than loDigit but strictly less than the
        // ceiling — that ends the key with room on both sides.
        const unsigned int mid = (loDigit + hiDigit) / 2;
        if (mid > loDigit) {
            out.push_back(mid);
            return fromDigits(out);                     // found a gap — done
        }

        // No gap at this position: we must copy loDigit and descend into a new segment.
        out.push_back(loDigit);
        // Placing loDigit keeps us below hi iff loDigit was strictly under hi's ceiling
        // (or we were already below). This lets the NEXT position use the full range.
        if (loDigit < hiDigit) belowHi = true;
    } //end for loop

    // The bounded loop ended without opening a gap: lo and hi are adjacent, so append a
    // fresh mid digit to form a strictly-larger dotted child (e.g. "5.1" → "5.1.I").
    out.push_back(static_cast<unsigned int>(kGenBase / 2));
    return fromDigits(out);
} //end method keyBetween

std::string keyBefore(std::string_view hi) { return keyBetween("0", hi); }
std::string keyAfter(std::string_view lo)  { return keyBetween(lo, "Z"); }

std::vector<std::string> rebalancedKeys(std::size_t n) {
    std::vector<std::string> out;
    if (n == 0) return out;

    // Single-digit spread across positions 1..(kGenBase-2) so there is room before the
    // first (0 is reserved) and after the last ('Z' == kGenBase-1 is the reserved open-top
    // sentinel, so the highest REAL key is 'Y' == kGenBase-2). Single digits need no dots.
    // Fits while n <= 34.
    const unsigned int lo = 1, hi = kGenBase - 2;                 // 1..34 ('1'..'Y')
    const unsigned int slots = hi - lo + 1;                       // 34
    if (n <= static_cast<std::size_t>(slots)) {
        int cur = lo;
        for (std::size_t i = 0; i < n; ++i) {
            // n == 1 is the divide-by-zero guard, NOT n == 0: the divisor is (n - 1), and
            // n == 0 already returned above. A lone key takes the midpoint so there is room
            // on both sides (I-0121). Integer division by zero is UB that traps on x86-64
            // (SIGFPE) but silently yields 0 on arm64 — green on Apple, red on Linux CI.
            const int want = (n == 1)
                ? (lo + hi) / 2
                : lo + static_cast<int>((static_cast<long long>(hi - lo)
                                         * static_cast<long long>(i))
                                        / static_cast<long long>(n - 1));
            int v = std::max(want, cur);
            if (v > hi) v = hi;
            out.push_back(std::string(1, genDigitChar(v)));
            cur = v + 1;
        } //end for
        return out;
    } //end if

    // Fallback for large n: two-segment dotted keys (e.g. "1.5"). First segment steps
    // 1..hi; second spreads within each bucket. Strictly ascending; each segment one digit.
    const std::size_t perFirst = (n + static_cast<std::size_t>(slots) - 1)
                                 / static_cast<std::size_t>(slots);
    std::size_t produced = 0;
    for (unsigned int a = lo; a <= hi && produced < n; ++a) {
        for (std::size_t j = 0; j < perFirst && produced < n; ++j) {
            const int b = lo + static_cast<unsigned int>(
                (static_cast<long long>(hi - lo) * static_cast<long long>(j))
                / static_cast<long long>(std::max<std::size_t>(1, perFirst)));
            std::string k;
            k += genDigitChar(a);
            k += '.';
            k += genDigitChar(std::max(static_cast<int>(b), static_cast<int>(lo)));
            out.push_back(k);
            ++produced;
        } //end inner for loop
    } //end outer for loop
    return out;
} //end method rebalance keys

bool isOrderKey(std::string_view s) {
    if (s.empty()) return false;
    if (s.front() == '.' || s.back() == '.') return false;   // no leading/trailing dot
    bool prevDot = false;
    for (char c : s) {
        if (c == '.') {
            if (prevDot) return false;                        // no empty segment ".."
            prevDot = true;
            continue;
        } //end if c == '.'
        prevDot = false;
        // Permissive acceptance: caps digits, or legacy lowercase (a-z) from pre-T-0358
        // dev projects, so their folders are never mistaken for un-migrated + reslugged.
        const bool ok = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
                        || (c >= 'a' && c <= 'z');
        if (!ok) return false;
    } //end for
    // A key ending in the lowest digit ('0') has no room to insert before the next key at
    // the same length; not canonical.
    return s.back() != '0';
} //end method isOrderKey

} // namespace scrivi::util
