// OrderKeyTests.cpp
// Unit tests for the fractional order-key generator (EP-027 P2, A4b). The critical
// property: keyBetween(lo,hi) always returns a key strictly between lo and hi, so
// repeated inserts between two neighbours never collide and always keep sort order ==
// manuscript order.

#include <catch2/catch_test_macros.hpp>

#include "util/OrderKey.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

using scrivi::util::keyBetween;
using scrivi::util::keyBefore;
using scrivi::util::keyAfter;
using scrivi::util::isOrderKey;
using scrivi::util::rebalancedKeys;

// A caps-only generated key uses ONLY 0-9 A-Z and the '.' segment separator (T-0358).
static bool isCapsOnly(const std::string& k) {
    if (k.empty()) return false;
    for (char c : k)
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || c == '.')) return false;
    return true;
}

TEST_CASE("keyBetween of two open bounds yields a valid mid key", "[OrderKey][EP-027]") {
    const std::string k = keyBetween("", "");
    REQUIRE_FALSE(k.empty());
    REQUIRE(isOrderKey(k));
}

TEST_CASE("keyBefore / keyAfter bracket a key", "[OrderKey][EP-027]") {
    const std::string mid = keyBetween("", "");
    const std::string before = keyBefore(mid);
    const std::string after  = keyAfter(mid);
    REQUIRE(before < mid);
    REQUIRE(mid < after);
    REQUIRE(isOrderKey(before));
    REQUIRE(isOrderKey(after));
}

TEST_CASE("keyBetween is strictly between ordered neighbours", "[OrderKey][EP-027]") {
    const std::string a = keyBetween("", "");            // some middle key
    const std::string b = keyAfter(a);                   // a < b
    REQUIRE(a < b);
    const std::string mid = keyBetween(a, b);
    REQUIRE(a < mid);
    REQUIRE(mid < b);
    REQUIRE(isOrderKey(mid));
}

TEST_CASE("repeated inserts between the SAME two neighbours never collide and stay ordered",
          "[OrderKey][EP-027]") {
    // The A4b invariant: insert 500 times between a fixed lo and the previously-inserted
    // key; every result must be strictly between and unique, and the whole set sorts.
    std::string lo = keyBefore(keyBetween("", ""));
    std::string hi = keyAfter(lo);
    REQUIRE(lo < hi);

    std::vector<std::string> keys;
    std::string right = hi;
    for (int i = 0; i < 500; ++i) {
        const std::string k = keyBetween(lo, right);
        REQUIRE(lo < k);
        REQUIRE(k < right);
        REQUIRE(isOrderKey(k));
        keys.push_back(k);
        right = k;   // next insert squeezes between lo and this new key
    }
    // All unique.
    std::vector<std::string> sorted = keys;
    std::sort(sorted.begin(), sorted.end());
    REQUIRE(std::unique(sorted.begin(), sorted.end()) == sorted.end());
}

TEST_CASE("a sequence of appends stays sorted", "[OrderKey][EP-027]") {
    // Simulate creating N chapters at the end (each keyAfter the last).
    std::vector<std::string> keys;
    std::string last;   // empty => before everything
    for (int i = 0; i < 200; ++i) {
        const std::string k = keyAfter(last);
        if (!keys.empty()) REQUIRE(keys.back() < k);
        REQUIRE(isOrderKey(k));
        keys.push_back(k);
        last = k;
    }
    std::vector<std::string> sorted = keys;
    std::sort(sorted.begin(), sorted.end());
    REQUIRE(sorted == keys);   // append order already sorted
}

TEST_CASE("interleaved front/middle/back inserts all stay globally sorted",
          "[OrderKey][EP-027]") {
    // Maintain an ordered vector; repeatedly insert at a computed position and assert the
    // vector remains strictly increasing.
    std::vector<std::string> v;
    v.push_back(keyBetween("", ""));
    for (int i = 0; i < 300; ++i) {
        const std::size_t pos = static_cast<std::size_t>((i * 7919) % (v.size() + 1));
        const std::string lo = pos == 0 ? std::string() : v[pos - 1];
        const std::string hi = pos == v.size() ? std::string() : v[pos];
        const std::string k = keyBetween(lo, hi);
        REQUIRE(isOrderKey(k));
        if (!lo.empty()) REQUIRE(lo < k);
        if (!hi.empty()) REQUIRE(k < hi);
        v.insert(v.begin() + static_cast<std::ptrdiff_t>(pos), k);
    }
    for (std::size_t i = 1; i < v.size(); ++i) {
        REQUIRE(v[i - 1] < v[i]);
    }
}

TEST_CASE("keyBetween equal bounds is empty; out-of-order bounds auto-repair",
          "[OrderKey][EP-027]") {
    // Equal bounds leave no room between them → empty (genuine misuse).
    REQUIRE(keyBetween("5", "5").empty());
    // Out-of-order bounds (lo > hi, both bounded) are auto-repaired by swapping, so the
    // caller still gets a valid key strictly between the two values.
    const std::string k = keyBetween("9", "1");
    REQUIRE_FALSE(k.empty());
    REQUIRE(std::string("1") < k);
    REQUIRE(k < std::string("9"));
    REQUIRE(isOrderKey(k));
}

// ---------------------------------------------------------------------------
// T-0358: caps-only generation + rebalance + legacy-lowercase compatibility
// ---------------------------------------------------------------------------

TEST_CASE("generated keys are caps-only base-36 (T-0358)", "[OrderKey][T-0358]") {
    // Every key keyBetween produces uses only 0-9 A-Z — never lowercase.
    std::string last;
    for (int i = 0; i < 200; ++i) {
        const std::string k = keyAfter(last);
        REQUIRE(isCapsOnly(k));
        last = k;
    }
    // Front + interior inserts, too.
    std::vector<std::string> v{ keyBetween("", "") };
    for (int i = 0; i < 200; ++i) {
        const std::size_t pos = static_cast<std::size_t>((i * 7919) % (v.size() + 1));
        const std::string lo = pos == 0 ? std::string() : v[pos - 1];
        const std::string hi = pos == v.size() ? std::string() : v[pos];
        const std::string k = keyBetween(lo, hi);
        REQUIRE(isCapsOnly(k));
        v.insert(v.begin() + static_cast<std::ptrdiff_t>(pos), k);
    }
}

TEST_CASE("caps generation is closed under inserts between caps neighbours (T-0358)",
          "[OrderKey][T-0358]") {
    // The post-rebalance world: all keys are caps. Every insert between two caps neighbours
    // — including appending past the top REAL key 'Y' (which grows to a dotted two-segment
    // key) — stays caps-only and strictly ordered. '0' and 'Z' are reserved open-bound
    // sentinels, never real keys. This is the invariant that keeps a project clean.
    const std::string mid = keyBetween("F", "I");
    REQUIRE(std::string("F") < mid); REQUIRE(mid < std::string("I"));
    REQUIRE(isCapsOnly(mid));

    // 'Z' is the reserved open-top sentinel: appending after it is undefined → empty.
    REQUIRE(keyAfter("Z").empty());

    const std::string afterY = keyAfter("Y");           // append past the top REAL single digit
    REQUIRE(std::string("Y") < afterY);
    REQUIRE(isCapsOnly(afterY));                         // dotted, e.g. "Y.I"

    const std::string beforeFirst = keyBefore("1");     // room before the first spread key
    REQUIRE(beforeFirst < std::string("1"));
    REQUIRE(isCapsOnly(beforeFirst));

    // Between a key and its dotted child: A < A.x < B, all consistent.
    const std::string child = keyBetween("A", "B");     // e.g. "A.H"? no — "A" and "B" have a
    REQUIRE(std::string("A") < child); REQUIRE(child < std::string("B"));
    const std::string grandchild = keyBetween("A", child);
    REQUIRE(std::string("A") < grandchild); REQUIRE(grandchild < child);
    REQUIRE(isCapsOnly(grandchild));
}

TEST_CASE("dotted keys sort identically under byte-sort and macOS natural-sort (T-0358)",
          "[OrderKey][T-0358]") {
    // The reason for the dot: each dot-segment is a single char, so no multi-digit run
    // exists for natural/numeric sort to reorder. We simulate natural sort (compare digit
    // runs numerically) and require it to agree with plain byte sort on generated keys.
    std::vector<std::string> keys;
    std::string last;
    for (int i = 0; i < 60; ++i) { last = keyAfter(last); keys.push_back(last); }
    // deep front-inserts to force dotted growth
    std::string lo = "A", hi = keyAfter("A");
    for (int i = 0; i < 40; ++i) { std::string k = keyBetween(lo, hi); keys.push_back(k); hi = k; }

    auto byteSorted = keys; std::sort(byteSorted.begin(), byteSorted.end());
    // Natural-sort comparator: compare char-by-char, but a maximal run of DIGITS compares
    // numerically. (macOS-style.)
    auto naturalLess = [](const std::string& a, const std::string& b) {
        std::size_t i = 0, j = 0;
        while (i < a.size() && j < b.size()) {
            bool da = std::isdigit((unsigned char)a[i]), db = std::isdigit((unsigned char)b[j]);
            if (da && db) {
                std::size_t i2 = i, j2 = j;
                while (i2 < a.size() && std::isdigit((unsigned char)a[i2])) ++i2;
                while (j2 < b.size() && std::isdigit((unsigned char)b[j2])) ++j2;
                long va = std::stol(a.substr(i, i2 - i)), vb = std::stol(b.substr(j, j2 - j));
                if (va != vb) return va < vb;
                i = i2; j = j2;
            } else {
                if (a[i] != b[j]) return a[i] < b[j];
                ++i; ++j;
            }
        }
        return a.size() < b.size();
    };
    auto natSorted = keys; std::sort(natSorted.begin(), natSorted.end(), naturalLess);
    REQUIRE(byteSorted == natSorted);   // dots make the two orders agree
}

TEST_CASE("legacy lowercase keys stay VALID even though generation is caps-only (T-0358)",
          "[OrderKey][T-0358]") {
    // Compatibility contract: a pre-T-0358 project's lowercase keys are accepted by
    // isOrderKey (so they are never mistaken for un-migrated folders + reslugged); the
    // one-time rebalance repair converts them to caps. Generation itself is caps-only and
    // is only used once neighbours are caps (see caps-closure test above).
    REQUIRE(isOrderKey("k"));
    REQUIRE(isOrderKey("aV"));
    REQUIRE(isOrderKey("z"));
}

TEST_CASE("rebalancedKeys returns short, spread, ascending, valid caps keys (T-0358)",
          "[OrderKey][T-0358]") {
    // Single-char spread tops out at 34 keys ('1'..'Y'); 0 and 'Z' are reserved sentinels.
    for (std::size_t n : {std::size_t(1), std::size_t(2), std::size_t(11),
                          std::size_t(34), std::size_t(100)}) {
        auto keys = rebalancedKeys(n);
        REQUIRE(keys.size() == n);
        for (std::size_t i = 0; i < keys.size(); ++i) {
            REQUIRE(isCapsOnly(keys[i]));
            REQUIRE(isOrderKey(keys[i]));
            if (i > 0) REQUIRE(keys[i - 1] < keys[i]);   // strictly ascending
        }
        // There is always room before the first and after the last (never starts/ends at
        // the extremes with no gap): a key can be inserted before keys[0] and after back().
        REQUIRE_FALSE(keyBefore(keys.front()).empty());
        REQUIRE_FALSE(keyAfter(keys.back()).empty());
        // For counts that fit the single-char spread, keys stay one char (legible).
        if (n <= 34) for (const auto& k : keys) REQUIRE(k.size() == 1);
    }
    REQUIRE(rebalancedKeys(0).empty());
}

TEST_CASE("rebalanced keys can be re-inserted between (spread leaves room) (T-0358)",
          "[OrderKey][T-0358]") {
    auto keys = rebalancedKeys(11);              // the-stairs case
    for (std::size_t i = 1; i < keys.size(); ++i) {
        const std::string mid = keyBetween(keys[i - 1], keys[i]);
        REQUIRE(keys[i - 1] < mid);
        REQUIRE(mid < keys[i]);
        REQUIRE(isCapsOnly(mid));
    }
}

TEST_CASE("isOrderKey rejects non-keys", "[OrderKey][EP-027]") {
    REQUIRE_FALSE(isOrderKey(""));        // empty
    REQUIRE_FALSE(isOrderKey("00"));      // ends in '0'
    REQUIRE_FALSE(isOrderKey("a0"));      // ends in '0'
    REQUIRE_FALSE(isOrderKey("a-b"));     // '-' is not in the alphabet
    REQUIRE_FALSE(isOrderKey(".A"));      // leading dot
    REQUIRE_FALSE(isOrderKey("A."));      // trailing dot
    REQUIRE_FALSE(isOrderKey("A..B"));    // empty segment
    REQUIRE(isOrderKey("a"));             // legacy lowercase still accepted (permissive)
    REQUIRE(isOrderKey("V1"));
    REQUIRE(isOrderKey("A.5"));           // dotted caps key (T-0358 canonical form)
    REQUIRE(isOrderKey("Q.3.T"));
    REQUIRE(isOrderKey("012"));           // all-digit key still valid
}
