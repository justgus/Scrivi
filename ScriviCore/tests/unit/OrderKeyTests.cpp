// OrderKeyTests.cpp
// Unit tests for the fractional order-key generator (EP-027 P2, A4b). The critical
// property: keyBetween(lo,hi) always returns a key strictly between lo and hi, so
// repeated inserts between two neighbours never collide and always keep sort order ==
// manuscript order.

#include <catch2/catch_test_macros.hpp>

#include "util/OrderKey.hpp"

#include <algorithm>
#include <string>
#include <vector>

using scrivi::util::keyBetween;
using scrivi::util::keyBefore;
using scrivi::util::keyAfter;
using scrivi::util::isOrderKey;

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

TEST_CASE("keyBetween on misuse (lo >= hi) returns empty", "[OrderKey][EP-027]") {
    REQUIRE(keyBetween("5", "5").empty());
    REQUIRE(keyBetween("9", "1").empty());
}

TEST_CASE("isOrderKey rejects non-keys", "[OrderKey][EP-027]") {
    REQUIRE_FALSE(isOrderKey(""));        // empty
    REQUIRE_FALSE(isOrderKey("00"));      // ends in '0'
    REQUIRE_FALSE(isOrderKey("a0"));      // ends in '0'
    REQUIRE_FALSE(isOrderKey("a-b"));     // '-' is not in the base-62 alphabet
    REQUIRE(isOrderKey("a"));
    REQUIRE(isOrderKey("V1"));
    // NOTE: a purely-numeric string like "001" IS a valid key (all base-62 digits, not
    // ending in '0'), so migration must NOT rely on isOrderKey to spot a legacy
    // `chapter-001` folder — it detects legacy folders by the zero-padded numeric slug
    // form directly (P3). Documented here so the P3 detector doesn't lean on isOrderKey.
    REQUIRE(isOrderKey("012"));
}
