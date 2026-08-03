#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

// Round-trips the scrivi_buffers_* C ABI through its JSON envelopes (EP-019,
// SP-056, T-0213). Buffers persist to <root>/history/buffers.json — a stateless
// read-modify-write with no open/close, so each test uses a UNIQUE temp root and
// removes it.

using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

JsonDoc okResult(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    JsonDoc env = std::move(parsed.value());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

JsonDoc envelope(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    return std::move(parsed.value());
}

// A unique temp project root that removes itself (incl. its history/ dir).
struct BuffersRoot {
    std::string path;
    BuffersRoot() {
        static std::atomic<int> counter{0};
        path = (std::filesystem::temp_directory_path() /
                ("scrivi-buffers-capi-" + std::to_string(counter.fetch_add(1)) + "-" +
                 std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ".scrivi"))
                   .string();
    }
    ~BuffersRoot() { std::error_code ec; std::filesystem::remove_all(path, ec); }
    const char* c() const { return path.c_str(); }
};

} // namespace

TEST_CASE("C ABI buffers: load then get round-trips text + present flag", "[BuffersCApi]") {
    BuffersRoot root;

    auto loaded = okResult(scrivi_buffers_load(root.c(), "1", "Kazd'ul", ""));
    REQUIRE(loaded.getString("bufferID") == "1");
    REQUIRE_FALSE(loaded.getString("updatedAt").empty());

    auto got = okResult(scrivi_buffers_get(root.c(), "1"));
    REQUIRE(got.getBool("present"));
    REQUIRE(got.getString("bufferID") == "1");
    REQUIRE(got.getString("text") == "Kazd'ul");
    REQUIRE_FALSE(got.getString("updatedAt").empty());
}

TEST_CASE("C ABI buffers: an unset slot reports present=false, empty text", "[BuffersCApi]") {
    BuffersRoot root;
    auto got = okResult(scrivi_buffers_get(root.c(), "7"));
    REQUIRE_FALSE(got.getBool("present"));
    REQUIRE(got.getString("text").empty());
}

TEST_CASE("C ABI buffers: load replaces an existing slot", "[BuffersCApi]") {
    BuffersRoot root;
    okResult(scrivi_buffers_load(root.c(), "2", "first", ""));
    okResult(scrivi_buffers_load(root.c(), "2", "second", ""));
    auto got = okResult(scrivi_buffers_get(root.c(), "2"));
    REQUIRE(got.getString("text") == "second");
}

TEST_CASE("C ABI buffers: a structured fragment round-trips through a slot (T-0355)",
          "[BuffersCApi][T-0355]") {
    BuffersRoot root;
    // A minimal scrivi.fragment.v1 object (shape not validated by the store — it is opaque
    // structure the editor round-trips to fragmentPaste).
    const char* frag =
        R"({"schema":"scrivi.fragment.v1","plainText":"a\n\nb",)"
        R"("pieces":[{"opensWith":"none","partial":"tail","text":"a"},)"
        R"({"opensWith":"scene","text":"b"}]})";

    okResult(scrivi_buffers_load(root.c(), "3", "a\n\nb", frag));

    auto got = okResult(scrivi_buffers_get(root.c(), "3"));
    REQUIRE(got.getBool("present"));
    REQUIRE(got.getString("text") == "a\n\nb");
    REQUIRE(got.contains("fragment"));                       // structured slot
    auto fragDoc = got.getSubDoc("fragment");
    REQUIRE(fragDoc.getString("schema") == "scrivi.fragment.v1");
    REQUIRE(fragDoc.arraySize("pieces") == 2);

    // list() also carries the fragment.
    auto listed = okResult(scrivi_buffers_list(root.c()));
    REQUIRE(listed.arraySize("buffers") == 1);
    REQUIRE(listed.arrayItem("buffers", 0).contains("fragment"));

    // Re-loading PLAIN text into the same slot clears the fragment (load replaces both).
    okResult(scrivi_buffers_load(root.c(), "3", "plain", ""));
    auto got2 = okResult(scrivi_buffers_get(root.c(), "3"));
    REQUIRE(got2.getString("text") == "plain");
    REQUIRE_FALSE(got2.contains("fragment"));                // fragment gone
}

TEST_CASE("C ABI buffers: list returns non-empty slots ascending, with a count",
          "[BuffersCApi]") {
    BuffersRoot root;
    okResult(scrivi_buffers_load(root.c(), "3", "gamma", ""));
    okResult(scrivi_buffers_load(root.c(), "1", "alpha", ""));
    okResult(scrivi_buffers_load(root.c(), "2", "beta", ""));

    auto listed = okResult(scrivi_buffers_list(root.c()));
    REQUIRE(listed.getInt("count") == 3);
    REQUIRE(listed.arraySize("buffers") == 3);
    REQUIRE(listed.arrayItem("buffers", 0).getString("bufferID") == "1");
    REQUIRE(listed.arrayItem("buffers", 1).getString("bufferID") == "2");
    REQUIRE(listed.arrayItem("buffers", 2).getString("bufferID") == "3");
    REQUIRE(listed.arrayItem("buffers", 1).getString("text") == "beta");
}

TEST_CASE("C ABI buffers: clear removes a slot; clearing an empty slot is a no-op",
          "[BuffersCApi]") {
    BuffersRoot root;
    okResult(scrivi_buffers_load(root.c(), "4", "delta", ""));

    auto cleared = okResult(scrivi_buffers_clear(root.c(), "4"));
    REQUIRE(cleared.getBool("cleared"));
    auto got = okResult(scrivi_buffers_get(root.c(), "4"));
    REQUIRE_FALSE(got.getBool("present"));

    auto again = okResult(scrivi_buffers_clear(root.c(), "4"));
    REQUIRE_FALSE(again.getBool("cleared"));   // already empty
}

TEST_CASE("C ABI buffers: buffers persist across a fresh load call (relaunch)",
          "[BuffersCApi]") {
    BuffersRoot root;
    okResult(scrivi_buffers_load(root.c(), "5", "epsilon", ""));
    // A second, independent call (no shared in-memory state) still sees the slot —
    // persistence is entirely on-disk in history/buffers.json.
    auto got = okResult(scrivi_buffers_get(root.c(), "5"));
    REQUIRE(got.getString("text") == "epsilon");
    // And the file exists where the design specifies.
    REQUIRE(std::filesystem::exists(
        std::filesystem::path(root.path) / "history" / "buffers.json"));
}

TEST_CASE("C ABI buffers: an out-of-range bufferID is rejected", "[BuffersCApi]") {
    BuffersRoot root;
    auto env0 = envelope(scrivi_buffers_load(root.c(), "0", "x", ""));
    REQUIRE_FALSE(env0.getBool("ok"));
    auto envA = envelope(scrivi_buffers_get(root.c(), "a"));
    REQUIRE_FALSE(envA.getBool("ok"));
    auto env10 = envelope(scrivi_buffers_clear(root.c(), "10"));
    REQUIRE_FALSE(env10.getBool("ok"));
}

TEST_CASE("C ABI buffers: a corrupt buffers.json is treated as empty, not an error",
          "[BuffersCApi]") {
    BuffersRoot root;
    // Write garbage where buffers.json lives.
    std::filesystem::create_directories(std::filesystem::path(root.path) / "history");
    {
        std::ofstream f(std::filesystem::path(root.path) / "history" / "buffers.json");
        f << "{ this is not valid json";
    }
    auto listed = okResult(scrivi_buffers_list(root.c()));   // ok, empty
    REQUIRE(listed.getInt("count") == 0);
    // A subsequent load overwrites the corrupt file cleanly.
    okResult(scrivi_buffers_load(root.c(), "6", "recovered", ""));
    auto got = okResult(scrivi_buffers_get(root.c(), "6"));
    REQUIRE(got.getString("text") == "recovered");
}
