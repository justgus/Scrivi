// I-0191 — appSupportRoot validation AT THE C ABI.
//
// ⚠️ WHY THIS FILE EXISTS. The defect lived at the boundary, not in the facade:
// `S(p) { return p ? p : ""; }` turned a NULL appSupportRoot into an empty
// string, which `join()` resolved as a RELATIVE path against the process working
// directory. The core then created its app-support skeleton in the CWD — the
// repository root, for a dev build — and reported SUCCESS. Three such
// directories, named from raw control bytes, were found in the repo on
// 2026-08-17.
//
// A facade test cannot see this: `S()` is an ABI-layer helper and the facade
// never calls it. Per the rule stated at the top of ObjectCApiTests.cpp, a test
// for a BOUNDARY change goes through scrivi_*.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "scrivi/Error.hpp"
#include "util/Json.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using scrivi::util::parseJson;

namespace {

// The literal name of one of the observed directories: \020 v \017 k \001.
const std::string kJunkBytes = std::string("\x10v\x0Fk\x01", 5);

std::vector<std::string> cwdEntries() {
    std::vector<std::string> names;
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(fs::current_path(), ec)) {
        names.push_back(e.path().filename().string());
    }
    std::sort(names.begin(), names.end());
    return names;
}

// Asserts the call was refused with invalidArgument, and left the CWD untouched.
void expectRejected(const char* raw, const std::vector<std::string>& before) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());

    auto& env = parsed.value();
    INFO("envelope: " << env.dump());

    // Must FAIL. Before the fix this returned ok:true having written to the CWD.
    REQUIRE_FALSE(env.getBool("ok"));

    auto err = env.getSubDoc("error");
    CHECK(err.getInt("code") ==
          static_cast<int>(scrivi::ErrorCode::invalidArgument));

    // And must have created nothing.
    CHECK(cwdEntries() == before);
    CHECK_FALSE(fs::exists(fs::path(kJunkBytes)));
    CHECK_FALSE(fs::exists("identity"));
    CHECK_FALSE(fs::exists("state"));
}

} // namespace

TEST_CASE("scrivi_ensure_local_identity - NULL appSupportRoot is rejected, not treated as \"\"",
          "[integration][cabi][I-0191]") {
    const auto before = cwdEntries();
    expectRejected(scrivi_ensure_local_identity("Ada", nullptr), before);
}

TEST_CASE("scrivi_ensure_local_identity - EMPTY appSupportRoot is rejected",
          "[integration][cabi][I-0191]") {
    const auto before = cwdEntries();
    expectRejected(scrivi_ensure_local_identity("Ada", ""), before);
}

TEST_CASE("scrivi_ensure_local_identity - appSupportRoot of unprintable bytes is rejected",
          "[integration][cabi][I-0191]") {
    const auto before = cwdEntries();
    expectRejected(scrivi_ensure_local_identity("Ada", kJunkBytes.c_str()), before);
}

TEST_CASE("scrivi_ensure_local_identity - RELATIVE appSupportRoot is rejected",
          "[integration][cabi][I-0191]") {
    const auto before = cwdEntries();
    expectRejected(scrivi_ensure_local_identity("Ada", "relative-app-support"), before);
}

TEST_CASE("scrivi_open_project - NULL appSupportRoot is rejected",
          "[integration][cabi][I-0191]") {
    const auto before = cwdEntries();
    expectRejected(scrivi_open_project("/tmp/nonexistent.scrivi", nullptr, "identity_x"),
                   before);
}

TEST_CASE("scrivi_create_project - junk appSupportRoot is rejected",
          "[integration][cabi][I-0191]") {
    const auto before = cwdEntries();
    expectRejected(scrivi_create_project("/tmp/nonexistent.scrivi",
                                         kJunkBytes.c_str(),
                                         "Title", "title",
                                         "identity_x", "persona_x", "Ada"),
                   before);
}
