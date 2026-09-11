// scrivi_make_test_world.cpp — SP-128 / T-0501 ([I-0195]) — FIXTURE BUILDER.
//
// ⚠️ NOT A TEST. It asserts nothing. It builds a deliberately LARGE throwaway
// project + world so the [I-0195] live pass has a load that legitimately takes a
// long time to open.
//
//   scrivi_make_test_world <projectDir> <appSupportRoot> [worldPackagePath] [scale]
//
// ⚠️ AN ARTIFICIAL DELAY WOULD NOT DO. A `sleep` in the read path would prove a
// progress bar animates; it would NOT exercise the real cost structure -- ONE
// `openScene` round trip PER SCENE, sequential, each crossing the mount -- which
// is what decides whether the fraction advances smoothly or stalls and jumps.
//
// ⚠️ TWO DIFFERENT KNOBS, and the live pass needs BOTH (Sprint SP-128 §2b):
//   - MANY SCENES makes the progress FRACTION MOVE (the bar counts scenes).
//   - A BULKY WORLD + a slow mount makes each step SLOW enough to watch.
// ⚠️ BULK ALONE ON LOCAL DISK IS FAST and shows nothing.
//
// ⚠️ DELIBERATELY Qt-FREE, like scrivi_world_probe. It links only ScriviCore, so
// it runs anywhere the core builds -- macOS workstation included, which is where
// a corpus is most conveniently generated before being copied to a slow mount.
//
// ⚠️ It REFUSES to touch an existing project directory. NEVER point it at real
// writing work (I-0150).

#include "scrivi/scrivi.h"

#include "DumasCorpus.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace scrivi::testcorpus;

// A scrivi_* result, freed on scope exit. ⚠️ The caller owns every returned
// pointer (scrivi.h) and leaking them across ~3,000 calls would matter.
class Owned {
public:
    explicit Owned(const char* p) : p_(p) {}
    ~Owned() { scrivi_free(p_); }
    Owned(const Owned&) = delete;
    Owned& operator=(const Owned&) = delete;
    [[nodiscard]] std::string_view view() const { return p_ ? p_ : ""; }
private:
    const char* p_;
};

// ⚠️ DELIBERATELY MINIMAL, and not a JSON parser. It lifts one string value from
// an envelope so the tool can chain calls. A real parser here would duplicate
// the core's own Json wrapper for no benefit -- and this file must stay Qt-free
// and dependency-free.
std::string field(std::string_view json, std::string_view key)
{
    const std::string needle = std::string("\"") + std::string(key) + "\"";
    const auto k = json.find(needle);
    if (k == std::string_view::npos) { return {}; }
    auto c = json.find(':', k + needle.size());
    if (c == std::string_view::npos) { return {}; }
    auto q1 = json.find('"', c);
    if (q1 == std::string_view::npos) { return {}; }
    auto q2 = json.find('"', q1 + 1);
    if (q2 == std::string_view::npos) { return {}; }
    return std::string(json.substr(q1 + 1, q2 - q1 - 1));
}

struct Group {
    const char*   kind;
    const char**  names;
    unsigned long count;
};

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::fprintf(stderr,
            "usage: scrivi_make_test_world <projectDir> <appSupportRoot> "
            "[worldPackagePath] [scale]\n");
        return 2;
    }
    const std::string projectPath = argv[1];
    const std::string appSupport  = argv[2];
    // ⚠️ The world is a SEPARATE PACKAGE, and the default places it as a SIBLING
    // of the project -- `<dir>/Dumas-France.scrivworld` next to
    // `<dir>/dumas.scrivi` -- NOT inside or under it.
    //
    // ⚠️ An earlier default appended ".worlds/" to the PROJECT path, which
    // implied the world lived inside the project. ✅ It never did, and for this
    // fixture the opposite is the point: the world is meant to be moved onto a
    // SLOW, SEPARATE VOLUME while the project stays on local disk. That is
    // exactly the [I-0195] arrangement the rig measured.
    const std::string worldPath   = (argc > 3 && *argv[3])
        ? std::string(argv[3])
        : (fs::path(projectPath).parent_path() / "Dumas-France.scrivworld").string();
    const int scale = (argc > 4) ? std::max(1, std::atoi(argv[4])) : 1;

    // ⚠️ REFUSES to touch an existing project (I-0150).
    if (fs::exists(projectPath)) {
        std::fprintf(stderr,
            "REFUSING: %s already exists. This fixture only CREATES projects.\n",
            projectPath.c_str());
        return 3;
    }

    const auto t0 = std::chrono::steady_clock::now();

    // --- identity ----------------------------------------------------------
    std::string identityID;
    std::string personaID;
    {
        Owned r{scrivi_ensure_local_identity("Dumas Fixture", appSupport.c_str())};
        identityID = field(r.view(), "identityID");
        // ⚠️ The key is `defaultPersonaID`, NOT `personaID`. Reading the wrong
        // one yields an EMPTY persona, and scrivi_create_project then refuses
        // the call -- which surfaces only as "could not create the project".
        personaID  = field(r.view(), "defaultPersonaID");
    }
    if (identityID.empty()) {
        std::fprintf(stderr, "FAIL: could not resolve a local identity under %s\n",
                     appSupport.c_str());
        return 1;
    }

    // --- project -----------------------------------------------------------
    std::string projectID;
    {
        Owned r{scrivi_create_project(projectPath.c_str(), appSupport.c_str(),
                                      "The France of Alexandre Dumas",
                                      "dumas-france",
                                      identityID.c_str(), personaID.c_str(),
                                      "Dumas Fixture")};
        projectID = field(r.view(), "projectID");
    }
    if (projectID.empty()) {
        std::fprintf(stderr, "FAIL: could not create the project\n");
        return 1;
    }

    // --- world -------------------------------------------------------------
    // ⚠️ MOST kinds are WORLD-scoped, so without a world this could only create
    // `source` objects -- and the point is bulk across many kinds.
    std::string worldID;
    {
        fs::create_directories(fs::path(worldPath).parent_path());
        Owned r{scrivi_create_world(projectPath.c_str(), worldPath.c_str(),
                                    "The France of Alexandre Dumas",
                                    "Anno Domini")};
        worldID = field(r.view(), "worldID");
    }
    if (worldID.empty()) {
        std::fprintf(stderr, "FAIL: could not create the world at %s\n",
                     worldPath.c_str());
        return 1;
    }
    std::printf("project: %s\nworld:   %s\n", projectPath.c_str(), worldPath.c_str());

    // --- objects -----------------------------------------------------------
    //
    // ⚠️ The kind STRINGS below are the core's own names, passed straight to
    // scrivi_create_object, which VALIDATES them. A kind the core does not know
    // is rejected there and simply produces no object -- so this cannot silently
    // invent a directory (CLAUDE.md's derived-not-restated rule).
    const Group groups[] = {
        {"character", kCharacters, countOf(kCharacters)},
        {"location",  kLocations,  countOf(kLocations)},
        {"building",  kBuildings,  countOf(kBuildings)},
        {"item",      kItems,      countOf(kItems)},
        {"faction",   kFactions,   countOf(kFactions)},
        {"vehicle",   kVehicles,   countOf(kVehicles)},
        {"map",       kMaps,       countOf(kMaps)},
        {"chronicle", kChronicles, countOf(kChronicles)},
        {"artifact",  kArtifacts,  countOf(kArtifacts)},
        {"rule",      kRules,      countOf(kRules)},
    };

    int objects = 0;
    std::vector<std::string> characterIDs;
    for (const Group& g : groups) {
        int made = 0;
        for (int rep = 0; rep < scale; ++rep) {
            for (unsigned long i = 0; i < g.count; ++i) {
                std::string name = g.names[i];
                if (rep > 0) { name += " (" + std::to_string(rep + 1) + ")"; }
                Owned r{scrivi_create_object(projectPath.c_str(), g.kind,
                                             name.c_str(), "",
                                             identityID.c_str(), personaID.c_str(),
                                             "Dumas Fixture", worldID.c_str())};
                const std::string id = field(r.view(), "objectID");
                if (id.empty()) { continue; }
                ++objects; ++made;
                if (std::strcmp(g.kind, "character") == 0) { characterIDs.push_back(id); }
            }
        }
        std::printf("objects: %-10s %4d   (running total %d)\n", g.kind, made, objects);
    }

    // --- manuscript --------------------------------------------------------
    //
    // ⚠️ THIS is what the progress bar counts: one `openScene` round trip per
    // scene. A big world behind few scenes still opens in a single jump.
    //
    // ⚠️ A chapter is born WITH its first scene, so each chapter contributes
    // 1 + (kSceneTitles - 1) scenes.
    int scenes = 0;
    std::string lastChapterID;
    for (int rep = 0; rep < scale; ++rep) {
        for (unsigned long c = 0; c < countOf(kChapters); ++c) {
            std::string chapterID;
            {
                Owned r{scrivi_create_chapter(projectPath.c_str(), appSupport.c_str(),
                                              projectID.c_str(),
                                              identityID.c_str(), personaID.c_str(),
                                              kChapters[c], lastChapterID.c_str())};
                chapterID = field(r.view(), "chapterID");
            }
            if (chapterID.empty()) { continue; }
            lastChapterID = chapterID;
            ++scenes;                       // the chapter's own first scene

            std::string lastSceneID;
            for (unsigned long s = 1; s < countOf(kSceneTitles); ++s) {
                Owned r{scrivi_create_scene(projectPath.c_str(), appSupport.c_str(),
                                            projectID.c_str(), chapterID.c_str(),
                                            lastSceneID.c_str(), "",
                                            identityID.c_str(), personaID.c_str(),
                                            kSceneTitles[s])};
                const std::string id = field(r.view(), "sceneID");
                if (id.empty()) { continue; }
                lastSceneID = id;
                ++scenes;
            }
        }
        std::printf("scenes:  running total %d\n", scenes);
    }

    // --- relationships -----------------------------------------------------
    //
    // ⚠️ Uses only a SEED relation type and RESPECTS its constraint:
    // `sibling-of` is character->character. A violated constraint is rejected by
    // the core, which would leave the graph thinner than the counts claim.
    int edges = 0;
    for (std::size_t i = 0; i + 1 < characterIDs.size(); i += 2) {
        Owned r{scrivi_create_edge(projectPath.c_str(),
                                   characterIDs[i].c_str(),
                                   characterIDs[i + 1].c_str(),
                                   "sibling-of", "")};
        if (!field(r.view(), "edgeID").empty()) { ++edges; }
    }

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t0).count();

    std::printf("\n== built in %lld ms ==\n", static_cast<long long>(ms));
    std::printf("objects:       %d\n", objects);
    std::printf("scenes:        %d   <- the progress bar counts THESE\n", scenes);
    std::printf("relationships: %d\n", edges);
    std::printf(
        "\n⚠️  BULK ALONE IS FAST. To make opening SLOW, put the WORLD on a\n"
        "    high-latency mount (the rig's cache=none,actimeo=1,closetimeo=1\n"
        "    cifs share) and open the project from the app.\n");
    return 0;
}
