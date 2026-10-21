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

TEST_CASE("C ABI buffers: a structured Range round-trips through a slot ([SP-191] Q4a)",
          "[BuffersCApi][sp191]") {
    BuffersRoot root;
    const char* range =
        R"({"schema":"scrivi.range.v1","scenes":[)"
        R"({"sceneID":"s1","chapterID":"c1","chapterTitle":"","sceneTitle":"","start":2,"length":1,"sceneLength":3,"text":"a"},)"
        R"({"sceneID":"s2","chapterID":"c1","chapterTitle":"","sceneTitle":"","start":0,"length":1,"sceneLength":4,"text":"b"}]})";

    okResult(scrivi_buffers_load(root.c(), "3", "a\n\nb", range));

    auto got = okResult(scrivi_buffers_get(root.c(), "3"));
    REQUIRE(got.getBool("present"));
    REQUIRE(got.getString("text") == "a\n\nb");
    REQUIRE(got.contains("range"));                          // structured slot
    auto rangeDoc = got.getSubDoc("range");
    REQUIRE(rangeDoc.getString("schema") == "scrivi.range.v1");
    REQUIRE(rangeDoc.arraySize("scenes") == 2);

    // list() also carries the range.
    auto listed = okResult(scrivi_buffers_list(root.c()));
    REQUIRE(listed.arraySize("buffers") == 1);
    REQUIRE(listed.arrayItem("buffers", 0).contains("range"));

    // Re-loading PLAIN text into the same slot clears the range (load replaces both).
    okResult(scrivi_buffers_load(root.c(), "3", "plain", ""));
    auto got2 = okResult(scrivi_buffers_get(root.c(), "3"));
    REQUIRE(got2.getString("text") == "plain");
    REQUIRE_FALSE(got2.contains("range"));                   // range gone
}

TEST_CASE("C ABI buffers: an old fragment is refused, and an old-style slot on disk reads as EMPTY ([SP-191] Q4a)",
          "[BuffersCApi][sp191]") {
    BuffersRoot root;
    const char* frag =
        R"({"schema":"scrivi.fragment.v1","plainText":"a\n\nb",)"
        R"("pieces":[{"opensWith":"none","partial":"tail","text":"a"},{"opensWith":"scene","text":"b"}]})";
    REQUIRE_FALSE(envelope(scrivi_buffers_load(root.c(), "3", "a\n\nb", frag)).getBool("ok"));
    REQUIRE_FALSE(okResult(scrivi_buffers_get(root.c(), "3")).getBool("present"));

    // A buffers file written before SP-191: slot 4 holds a fragment, slot 5 plain text.
    okResult(scrivi_buffers_load(root.c(), "5", "kept", ""));
    const std::filesystem::path file = std::filesystem::path(root.path) / "history" / "buffers.json";
    std::ofstream(file, std::ios::binary)
        << R"({"schema":"scrivi.buffers.v1","buffers":[)"
        << R"({"bufferID":"4","text":"a\n\nb","updatedAt":"2026-10-01T00:00:00Z","fragment":)" << frag << "},"
        << R"({"bufferID":"5","text":"kept","updatedAt":"2026-10-01T00:00:00Z"}]})";
    REQUIRE_FALSE(okResult(scrivi_buffers_get(root.c(), "4")).getBool("present"));   // pretend it is empty
    auto listed = okResult(scrivi_buffers_list(root.c()));
    REQUIRE(listed.getInt("count") == 1);
    REQUIRE(listed.arrayItem("buffers", 0).getString("bufferID") == "5");

    // The next write drops it from disk.
    okResult(scrivi_buffers_load(root.c(), "6", "new", ""));
    std::ifstream in(file, std::ios::binary);
    const std::string onDisk{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    REQUIRE(onDisk.find("fragment") == std::string::npos);
    REQUIRE(onDisk.find("\"kept\"") != std::string::npos);
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
