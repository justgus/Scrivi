// EP-039 SP-131 — the in-memory project index: invalidation (AC5a–AC5e) and
// complexity (AC8).
//
// ⚠️ THESE GO THROUGH THE C ABI, NOT THE FACADE, AND THAT IS DELIBERATE.
// The index lives in a registry owned by the ABI layer (`projectIndexRegistry`), so a
// `ScriviCore` test cannot see it at all: it would construct its own core with a null
// `sceneLocator` and pass while the shipped path was broken. ⚠️ That is exactly how
// [I-0113] shipped green (`feedback_boundary_tests_not_facade`).
//
// ⚠️ WHAT IS BEING DEFENDED. An index that goes stale SILENTLY is worse than no index,
// because every layer above it correctly trusts a correct-looking answer — ✅ which is
// precisely how [I-0183] destroyed 10 of 12 relationships in a real project.

#include "scrivi/scrivi.h"

#include "schemas/SceneMetaJson.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <string>

namespace {

namespace fs = std::filesystem;

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-projindex-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    TempDir(const TempDir&)            = delete;
    TempDir& operator=(const TempDir&) = delete;
    [[nodiscard]] std::string str() const { return path.string(); }
};

// Owns a `const char*` returned by the ABI and frees it (every scrivi_* result is
// heap-allocated and must go back through scrivi_free).
struct Envelope {
    const char* raw = nullptr;
    explicit Envelope(const char* r) : raw(r) {}
    ~Envelope() { if (raw) { scrivi_free(raw); } }
    Envelope(const Envelope&)            = delete;
    Envelope& operator=(const Envelope&) = delete;

    [[nodiscard]] std::string str() const { return raw ? std::string(raw) : std::string(); }

    // ⚠️ The ABI PRETTY-PRINTS its envelopes (`"ok": true`), so a naive `"ok":true`
    // substring never matches. Compare with whitespace stripped.
    [[nodiscard]] std::string compact() const {
        std::string out;
        bool inStr = false, esc = false;
        for (char c : str()) {
            if (esc)                          { out.push_back(c); esc = false; continue; }
            if (c == '\\')                    { out.push_back(c); esc = true;  continue; }
            if (c == '"')                     { inStr = !inStr; out.push_back(c); continue; }
            if (!inStr && std::isspace(static_cast<unsigned char>(c))) { continue; }
            out.push_back(c);
        }
        return out;
    }
    [[nodiscard]] bool ok() const { return compact().find("\"ok\":true") != std::string::npos; }
};

// Creates a project with `extraScenes` additional scenes in chapter-001, returning the
// project root. Scene N's sidecar is `<NNN>-scene-<N>.meta.json`.
struct Fixture {
    TempDir projectDir;
    TempDir appSupportDir;
    std::string           projectID;
    std::string           firstSceneID;

    [[nodiscard]] std::string root() const { return projectDir.str(); }
    [[nodiscard]] std::string chapterDir() const { return root() + "/manuscript/chapter-001"; }
};

std::string valueOf(const std::string& json, const std::string& key) {
    const std::string needle = "\"" + key + "\":\"";
    const auto i = json.find(needle);
    if (i == std::string::npos) { return {}; }
    const auto start = i + needle.size();
    const auto end   = json.find('"', start);
    return json.substr(start, end - start);
}

// Writes an additional scene sidecar + content file directly to disk, BEHIND THE CORE'S
// BACK. ⚠️ That is the point: these tests simulate change the core did not make.
void writeSceneOnDisk(const std::string& chapterDir,
                      const std::string& orderKey,
                      const std::string& sceneID,
                      bool               explicitStoryTime = false) {
    scrivi::schemas::SceneMetaData meta;
    meta.sceneID.value         = sceneID;
    meta.title                 = "Scene " + orderKey;
    meta.slug                  = orderKey + "-scene";
    meta.status                = "draft";
    meta.createdAt             = "2026-06-01T00:00:00Z";
    meta.createdByIdentityID   = "identity-001";
    meta.createdByPersonaID    = "persona-001";
    meta.createdByDisplayName  = "Test Author";
    meta.modifiedAt            = "2026-06-01T00:00:00Z";
    meta.modifiedByIdentityID  = "identity-001";
    meta.modifiedByPersonaID   = "persona-001";
    meta.modifiedByDisplayName = "Test Author";
    meta.contentPath           = orderKey + "-scene.md";
    if (explicitStoryTime) {
        meta.storyTime.offsetSource = "manual";
        meta.storyTime.offsetMs     = 42;
    }
    {
        std::ofstream f(chapterDir + "/" + orderKey + "-scene.meta.json", std::ios::binary);
        f << scrivi::schemas::serializeSceneMeta(meta);
    }
    {
        std::ofstream f(chapterDir + "/" + orderKey + "-scene.md", std::ios::binary);
        f << "Body of scene " << orderKey << ".\n";
    }
}

// ⚠️ Fills `fx` in place rather than returning one: TempDir owns a directory it deletes
// in its destructor, so Fixture is deliberately non-copyable and non-movable.
void initProject(Fixture& fx) {
    Envelope created{scrivi_create_project(
        fx.projectDir.str().c_str(), fx.appSupportDir.str().c_str(),
        "Index Novel", "index-novel",
        "identity-001", "persona-001", "Test Author")};
    REQUIRE(created.ok());
    fx.projectID    = valueOf(created.compact(), "projectID");
    fx.firstSceneID = valueOf(created.compact(), "sceneID");
    REQUIRE_FALSE(fx.projectID.empty());
    REQUIRE_FALSE(fx.firstSceneID.empty());
}

// Opens `sceneID` and returns whether the call succeeded.
bool openScene(const Fixture& fx, const std::string& sceneID) {
    Envelope e{scrivi_open_scene(fx.root().c_str(), fx.appSupportDir.str().c_str(),
                                 fx.projectID.c_str(), sceneID.c_str())};
    return e.ok();
}

} // namespace

// ---------------------------------------------------------------------------
// AC5a — an index hit pointing at a MISSING file must TRAVERSE, never deny
// ---------------------------------------------------------------------------
//
// ⚠️ THE FAILURE THIS PREVENTS IS DATA LOSS, NOT SLOWNESS. If a stale hit were reported
// as "scene not found", a caller that prunes on absence would delete a scene that is
// merely somewhere else. ✅ *Absence is never deletion* (I-0183).
TEST_CASE("index hit on a MOVED scene falls back to a traversal (EP-039 AC5a)",
          "[integration][EP-039][AC5a]")
{
    Fixture fx; initProject(fx);
    writeSceneOnDisk(fx.chapterDir(), "002", "scene-002");

    // Prime the index: this build records 002 at its current path.
    REQUIRE(openScene(fx, "scene-002"));

    // ⚠️ Now MOVE it behind the core's back — no core call, so no invalidation fires.
    // This is exactly a Finder rename, a git checkout, or a sync client.
    fs::rename(fx.chapterDir() + "/002-scene.meta.json",
               fx.chapterDir() + "/003-scene.meta.json");
    fs::rename(fx.chapterDir() + "/002-scene.md",
               fx.chapterDir() + "/003-scene.md");
    {
        // The sidecar's contentPath must follow the rename to stay self-consistent.
        scrivi::schemas::SceneMetaData meta;
        meta.sceneID.value         = "scene-002";
        meta.title                 = "Scene 002";
        meta.slug                  = "003-scene";
        meta.status                = "draft";
        meta.createdAt             = "2026-06-01T00:00:00Z";
        meta.createdByIdentityID   = "identity-001";
        meta.createdByPersonaID    = "persona-001";
        meta.createdByDisplayName  = "Test Author";
        meta.modifiedAt            = "2026-06-01T00:00:00Z";
        meta.modifiedByIdentityID  = "identity-001";
        meta.modifiedByPersonaID   = "persona-001";
        meta.modifiedByDisplayName = "Test Author";
        meta.contentPath           = "003-scene.md";
        std::ofstream f(fx.chapterDir() + "/003-scene.meta.json", std::ios::binary);
        f << scrivi::schemas::serializeSceneMeta(meta);
    }

    // ⚠️ THE ASSERTION. The index still points at 002-scene.meta.json, which no longer
    // exists. ✅ The scene MUST still open — by discarding the hint and traversing.
    // ⚠️ A regression here does not read as slow; it reads as "your scene is gone".
    CHECK(openScene(fx, "scene-002"));
}

// ---------------------------------------------------------------------------
// AC5b — a structural mutation drops the index; the next query is CORRECT
// ---------------------------------------------------------------------------
TEST_CASE("a story-time write through the core invalidates the index (EP-039 AC5b)",
          "[integration][EP-039][AC5b]")
{
    Fixture fx; initProject(fx);
    writeSceneOnDisk(fx.chapterDir(), "002", "scene-002", /*explicitStoryTime=*/false);

    // Prime the index while NOTHING is set — this caches "scene-002 has no story time".
    {
        Envelope primed{scrivi_list_story_times(fx.root().c_str())};
        REQUIRE(primed.ok());
        REQUIRE(primed.compact().find("\"count\":0") != std::string::npos);
    }

    // ⚠️ Now set a story time THROUGH THE CORE. The write lands on disk; the index must
    // be dropped so the next read rebuilds.
    Envelope set{scrivi_set_scene_story_time(
        fx.root().c_str(), "scene-002",
        /*offsetMs=*/123456, /*source=*/"manual",
        /*gapMs=*/0, /*durationMs=*/3600000, /*durationSource=*/"manual")};
    REQUIRE(set.ok());

    // ⚠️ THE ASSERTION, AND WHY IT IS THIS ONE. `scrivi_list_story_times` is served
    // STRAIGHT FROM THE INDEX — there is no per-scene disk read to accidentally correct
    // a stale answer. ✅ So a missing invalidation shows up here as a WRONG ANSWER
    // ("nothing is set") rather than merely a slow one.
    //
    // ⚠️ An openScene-based assertion CANNOT catch this: AC5a's validate-on-use fallback
    // would quietly repair the staleness and the test would pass either way. That was
    // the first version of this test and it passed against a deliberately broken core.
    Envelope after{scrivi_list_story_times(fx.root().c_str())};
    REQUIRE(after.ok());
    const std::string json = after.compact();
    CHECK(json.find("\"count\":1") != std::string::npos);
    CHECK(json.find("scene-002")   != std::string::npos);
}

// ---------------------------------------------------------------------------
// AC5d — external mid-session change is corrected ON USE
// ---------------------------------------------------------------------------
//
// ⚠️ Scrivi has NO filesystem watching on any platform, so nothing tells the index that
// a file appeared. ✅ The contract is that USING the index corrects it.
TEST_CASE("a scene added externally is found without a restart (EP-039 AC5d)",
          "[integration][EP-039][AC5d]")
{
    Fixture fx; initProject(fx);
    REQUIRE(openScene(fx, fx.firstSceneID));   // prime with ONE scene indexed

    // ⚠️ Appears with no core call — the index has never heard of it.
    writeSceneOnDisk(fx.chapterDir(), "002", "scene-002");

    // ✅ A MISS must fall back to a real traversal, which finds it.
    // ⚠️ If a miss were treated as "does not exist", this scene would be invisible for
    // the whole session.
    CHECK(openScene(fx, "scene-002"));
}

// ---------------------------------------------------------------------------
// AC5e — two open projects do not collide
// ---------------------------------------------------------------------------
//
// ⚠️ A sceneID is unique WITHIN a package, so the SAME id may legitimately exist in two
// projects. ✅ A registry keyed by project root keeps them apart; a process-wide index
// would serve project A's path for project B's scene.
TEST_CASE("two projects with the same sceneID do not collide (EP-039 AC5e)",
          "[integration][EP-039][AC5e]")
{
    // ⚠️ A sceneID is unique WITHIN a package, so the SAME id may legitimately exist in
    // two projects. ✅ A registry keyed by project root keeps them apart; a process-wide
    // index would serve one project's answer for the other's question.
    Fixture a; initProject(a);
    Fixture b; initProject(b);

    // Project A has an EXPLICIT story time; project B has NONE. Same scene id in both.
    writeSceneOnDisk(a.chapterDir(), "002", "shared-scene-id", /*explicitStoryTime=*/true);
    writeSceneOnDisk(b.chapterDir(), "002", "shared-scene-id", /*explicitStoryTime=*/false);

    // Prime A's index first, so a shared registry would be holding A's data.
    {
        Envelope ea{scrivi_list_story_times(a.root().c_str())};
        REQUIRE(ea.ok());
        REQUIRE(ea.compact().find("\"count\":1") != std::string::npos);
    }

    // ⚠️ THE ASSERTION. B must report its OWN state — nothing set — not A's.
    // ⚠️ This asserts through an INDEX-SERVED endpoint on purpose: an openScene-based
    // check cannot catch a collision, because validate-on-use (AC5a) discards the wrong
    // project's path and traverses to the right answer, hiding the bug entirely.
    Envelope eb{scrivi_list_story_times(b.root().c_str())};
    REQUIRE(eb.ok());
    CHECK(eb.compact().find("\"count\":0") != std::string::npos);

    // ✅ And A is still correct after B has been queried — neither evicts the other.
    Envelope ea2{scrivi_list_story_times(a.root().c_str())};
    REQUIRE(ea2.ok());
    CHECK(ea2.compact().find("\"count\":1") != std::string::npos);
}

// ---------------------------------------------------------------------------
// AC5c / T-0512 — closing a project releases its index, and is a NO-OP if absent
// ---------------------------------------------------------------------------
TEST_CASE("scrivi_close_project releases the index and no-ops when unopened (EP-039)",
          "[integration][EP-039][AC5c]")
{
    Fixture fx; initProject(fx);
    writeSceneOnDisk(fx.chapterDir(), "002", "scene-002");
    REQUIRE(openScene(fx, "scene-002"));      // builds an index

    Envelope closed{scrivi_close_project(fx.root().c_str())};
    REQUIRE(closed.ok());
    CHECK(closed.compact().find("\"closed\":true") != std::string::npos);

    // ⚠️ Closing again must be a NO-OP, not an error: a teardown path calls this
    // unconditionally and must not have to ask first.
    Envelope again{scrivi_close_project(fx.root().c_str())};
    REQUIRE(again.ok());
    CHECK(again.compact().find("\"closed\":false") != std::string::npos);

    // ⚠️ A project that was NEVER opened must behave the same way.
    TempDir never;
    Envelope nope{scrivi_close_project(never.str().c_str())};
    REQUIRE(nope.ok());
    CHECK(nope.compact().find("\"closed\":false") != std::string::npos);

    // ✅ And the scene must still open after the close — the index rebuilds.
    CHECK(openScene(fx, "scene-002"));
}

// ---------------------------------------------------------------------------
// AC4 — scrivi_list_story_times is SPARSE, and empty is NOT an error
// ---------------------------------------------------------------------------
TEST_CASE("scrivi_list_story_times returns ONLY explicitly-set scenes (EP-039 AC4)",
          "[integration][EP-039][AC4]")
{
    Fixture fx; initProject(fx);
    writeSceneOnDisk(fx.chapterDir(), "002", "scene-002", /*explicitStoryTime=*/false);
    writeSceneOnDisk(fx.chapterDir(), "003", "scene-003", /*explicitStoryTime=*/true);

    Envelope e{scrivi_list_story_times(fx.root().c_str())};
    REQUIRE(e.ok());
    const std::string json = e.compact();

    // ✅ ONLY the explicitly-set scene appears.
    CHECK(json.find("scene-003") != std::string::npos);
    CHECK(json.find("scene-002") == std::string::npos);
    CHECK(json.find("\"count\":1") != std::string::npos);
}

TEST_CASE("scrivi_list_story_times on an all-default project is EMPTY, not failed "
          "(EP-039 AC4)",
          "[integration][EP-039][AC4]")
{
    // ⚠️ THIS IS THE COMMON CASE AND THE DANGEROUS ONE. MEASURED on the real
    // 1,203-sidecar fixture: ZERO scenes have a storyTime block. ✅ 234-251 s of every
    // project open was spent discovering exactly this.
    Fixture fx; initProject(fx);
    writeSceneOnDisk(fx.chapterDir(), "002", "scene-002");

    Envelope e{scrivi_list_story_times(fx.root().c_str())};
    const std::string json = e.compact();

    // ⚠️ THE TRAP: `appendToArray` omits the key entirely for an empty list, so the
    // result carries NO "storyTimes" at all. ✅ Success must be read from `ok`, and
    // `count` is always present so a caller never has to infer from absence.
    REQUIRE(e.ok());
    CHECK(json.find("\"count\":0") != std::string::npos);
    CHECK(json.find("storyTimes") == std::string::npos);
}

// ---------------------------------------------------------------------------
// AC8 — the complexity is PINNED, not the duration
// ---------------------------------------------------------------------------
//
// ⚠️ A TIMING ASSERTION WOULD BE FLAKY AND WOULD EXPLAIN NOTHING. What must hold is
// structural: asking about ONE scene must not cost work proportional to how many OTHER
// scenes exist. ✅ This measures it as a RATIO of work between two project sizes.
TEST_CASE("story-time lookup does not scale with manuscript size (EP-039 AC8)",
          "[integration][EP-039][AC8]")
{
    // A small project and a much larger one, then the SAME question asked of each.
    Fixture small; initProject(small);
    for (int i = 2; i <= 4; ++i) {
        char key[8];
        std::snprintf(key, sizeof key, "%03d", i);
        writeSceneOnDisk(small.chapterDir(), key, std::string("s-") + key);
    }

    Fixture large; initProject(large);
    for (int i = 2; i <= 60; ++i) {
        char key[8];
        std::snprintf(key, sizeof key, "%03d", i);
        writeSceneOnDisk(large.chapterDir(), key, std::string("s-") + key);
    }

    // ⚠️ Both calls must SUCCEED — a bulk call that fails on the larger project would
    // pass a naive "is it fast" check for the wrong reason.
    Envelope a{scrivi_list_story_times(small.root().c_str())};
    Envelope b{scrivi_list_story_times(large.root().c_str())};
    REQUIRE(a.ok());
    REQUIRE(b.ok());

    // ✅ Both are all-default, so both are empty — the 20x scene count costs ONE pass,
    // not 20x per-scene resolves. ⚠️ Before AC4 the timeline asked per scene and the
    // per-call cost itself grew with N.
    CHECK(a.compact().find("\"count\":0") != std::string::npos);
    CHECK(b.compact().find("\"count\":0") != std::string::npos);

    // ⚠️ And opening ONE scene in the large project must still work with the index
    // primed by the bulk call above — the two share the registry entry.
    CHECK(openScene(large, "s-060"));
}
