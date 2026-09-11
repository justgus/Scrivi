// dumas_world_fixture — SP-128 / T-0501 ([I-0195]) — builds a LARGE test world.
//
// ⚠️ THIS IS A FIXTURE BUILDER, NOT A TEST. It asserts nothing. It exists so the
// [I-0195] live pass runs against a project that legitimately takes a long time
// to open, because the defect ONLY appears under a load that is genuinely slow.
//
// ⚠️ AN ARTIFICIAL DELAY WOULD NOT DO. A `sleep` in the read path would prove a
// progress bar animates; it would NOT exercise the real cost structure -- ONE
// `openScene` round trip PER SCENE, sequential, each crossing the mount -- which
// is what decides whether the fraction advances smoothly or stalls and jumps.
//
// ⚠️ TWO DIFFERENT KNOBS, and the live pass needs BOTH (Sprint SP-128 §2b):
//   - MANY SCENES makes the progress FRACTION MOVE (the bar counts scenes).
//   - A BULKY WORLD + a slow mount makes each step SLOW enough to watch.
// A world of thousands of objects behind five scenes still opens 0->100% in one
// jump, which would show nothing.
//
// ---------------------------------------------------------------------------
// The setting: ALEXANDRE DUMAS' FRANCE (public domain -- Dumas died in 1870).
//
// Chosen over invented filler because the three novels share ONE France, so the
// relationships are REAL rather than random: the same king, the same Paris, the
// same regiment. `Le Comte de Monte-Cristo` (1844), `Les Trois Mousquetaires`
// (1844) and `Le Vicomte de Bragelonne` (1847-50, whose final part is known in
// English as `The Man in the Iron Mask`).
//
// ⚠️ NAMES ARE FROM THE NOVELS, which are public domain. No modern adaptation,
// translation-specific coinage, or invented "expansion" material is used.
// ---------------------------------------------------------------------------
//
// ⚠️ KINDS ARE DERIVED FROM THE CORE via ObjectKindScope -- this file names no
// kind list of its own and does not become another restatement of the standing
// rule (CLAUDE.md: "kind lists must be DERIVED, never restated").
//
// ⚠️ NEVER point this at real writing work (I-0150,
// feedback_never_drive_synthetic_input_at_real_work). It CREATES a throwaway
// project and refuses to touch one that already exists.
//
//   argv[1] = project dir to create      (required)
//   argv[2] = world package path         (optional; default alongside the project)
//   argv[3] = scale factor, default 1    (2 = twice the scenes and objects, ...)

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include "AppSupport.hpp"
#include "ObjectKindScope.hpp"
#include "ScriviBridge.hpp"

#include "DumasCorpus.hpp"

namespace {

// --- The Dumas corpus ------------------------------------------------------
//
// ⚠️ ONE CORPUS, SHARED. The names live in ScriviCore/tools/DumasCorpus.hpp so
// this Qt fixture and the Qt-free native tool (scrivi_make_test_world) can never
// drift into two different worlds.
using namespace scrivi::testcorpus;

// Find the core's own name for a kind by matching the tail of its subdir or the
// kind string itself. ⚠️ Returns empty when the core does not know it -- the
// caller then SKIPS that group rather than inventing a directory.
QString kindNamed(ScriviBridge* bridge, const QString& want)
{
    for (const auto& k : ObjectKindScope::kinds(bridge)) {
        if (k.kind.compare(want, Qt::CaseInsensitive) == 0) { return k.kind; }
    }
    return {};
}

struct Group {
    const char*  wanted;      // the kind we HOPE the core has
    const char** names;
    std::size_t  count;
};

}  // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr,
            "usage: dumas_world_fixture <projectDir> [worldPackagePath] [scale]\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString worldPath   = argc > 2 && *argv[2]
        ? QString::fromUtf8(argv[2])
        : projectPath + QStringLiteral(".worlds/Dumas-France.scrivworld");
    const int scale = argc > 3 ? qMax(1, QString::fromUtf8(argv[3]).toInt()) : 1;

    // ⚠️ REFUSES to touch an existing project. This builds throwaway data and
    // must never be pointed at real writing work (I-0150).
    if (QDir(projectPath).exists()) {
        std::fprintf(stderr,
            "REFUSING: %s already exists. This fixture only CREATES projects.\n",
            qPrintable(projectPath));
        return 3;
    }

    const QString appSupport = scrivi::linux_app::appSupportRoot();
    QElapsedTimer clock;
    clock.start();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Dumas Fixture"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: bridge did not bootstrap\n");
        return 1;
    }

    const QVariantMap created = bridge.createProject(
        projectPath, appSupport,
        QStringLiteral("The France of Alexandre Dumas"),
        QStringLiteral("dumas-france"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: could not create the project\n");
        return 1;
    }

    // ⚠️ MOST kinds are WORLD-scoped, so without a world this fixture could only
    // create `source` objects -- and the whole point is bulk across many kinds.
    const QVariantMap world = bridge.createWorld(
        projectPath, worldPath,
        QStringLiteral("The France of Alexandre Dumas"),
        QStringLiteral("Anno Domini"));
    const QString worldID = world.value(QStringLiteral("worldID")).toString();
    if (worldID.isEmpty()) {
        std::fprintf(stderr, "FAIL: could not create the world at %s\n",
                     qPrintable(worldPath));
        return 1;
    }
    std::printf("world:   %s\n", qPrintable(worldPath));
    std::printf("project: %s\n", qPrintable(projectPath));

    // --- Objects -----------------------------------------------------------
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
    QStringList characterIDs;
    QStringList locationIDs;
    for (const Group& g : groups) {
        // ⚠️ DERIVED: if the core does not know this kind, SKIP it rather than
        // writing a directory the core never named.
        const QString kind = kindNamed(&bridge, QString::fromLatin1(g.wanted));
        if (kind.isEmpty()) {
            std::printf("skip:    core does not know kind '%s'\n", g.wanted);
            continue;
        }
        const bool worldScoped = ObjectKindScope::isWorldScoped(&bridge, kind);
        for (int rep = 0; rep < scale; ++rep) {
            for (std::size_t i = 0; i < g.count; ++i) {
                QString name = QString::fromUtf8(g.names[i]);
                // Scale duplicates get a suffix so names stay distinct.
                if (rep > 0) { name += QStringLiteral(" (%1)").arg(rep + 1); }
                const QVariantMap r = bridge.createObject(
                    projectPath, kind, name, QString(),
                    worldScoped ? worldID : QString());
                const QString id = r.value(QStringLiteral("objectID")).toString();
                if (id.isEmpty()) { continue; }
                ++objects;
                if (kind == QStringLiteral("character")) { characterIDs << id; }
                if (kind == QStringLiteral("location"))  { locationIDs  << id; }
            }
        }
        std::printf("objects: %-10s running total %d\n", qPrintable(kind), objects);
    }

    // --- Manuscript --------------------------------------------------------
    //
    // ⚠️ THIS is what the progress bar counts: one `openScene` round trip per
    // scene. A big world behind few scenes would still open in one jump.
    const QVariantMap opened = bridge.openProject(projectPath, appSupport);
    QString lastChapterID =
        opened.value(QStringLiteral("scenes")).toList().isEmpty()
            ? QString()
            : opened.value(QStringLiteral("scenes")).toList().at(0).toMap()
                  .value(QStringLiteral("chapterID")).toString();

    int scenes = 0;
    for (int rep = 0; rep < scale; ++rep) {
        for (std::size_t c = 0; c < countOf(kChapters); ++c) {
            (void)c;
            // ⚠️ ScriviBridge::createChapter takes NO display name -- the core
            // names a chapter itself, and a chapter is born WITH its first
            // scene. The kChapters/kSceneTitles tables therefore document the
            // intended shape of the corpus; the on-disk titles come from the
            // core. ⚠️ Do NOT "fix" this by writing titles directly to disk:
            // the filesystem layout is the core's (EP-027).
            const QVariantMap ch = bridge.createChapter(
                projectPath, appSupport, projectID, lastChapterID);
            const QString chapterID = ch.value(QStringLiteral("chapterID")).toString();
            if (chapterID.isEmpty()) { continue; }
            lastChapterID = chapterID;
            ++scenes;              // the chapter's own first scene

            QString lastSceneID;
            for (std::size_t s = 1; s < countOf(kSceneTitles); ++s) {
                const QVariantMap sc = bridge.createScene(
                    projectPath, appSupport, projectID, chapterID, lastSceneID);
                const QString sceneID = sc.value(QStringLiteral("sceneID")).toString();
                if (sceneID.isEmpty()) { continue; }
                lastSceneID = sceneID;
                ++scenes;
            }
        }
        std::printf("scenes:  running total %d\n", scenes);
    }

    // --- Relationships -----------------------------------------------------
    //
    // ⚠️ Uses only the SEED relation types and RESPECTS their constraints:
    // `sibling-of` is character->character, `located-at` is scene->location.
    // A violated constraint is rejected by the core, which would leave the graph
    // thinner than the counts here suggest.
    int edges = 0;
    for (int i = 0; i + 1 < characterIDs.size(); i += 2) {
        const QVariantMap e = bridge.createEdge(
            projectPath, characterIDs.at(i), characterIDs.at(i + 1),
            QStringLiteral("sibling-of"), QString());
        if (!e.value(QStringLiteral("edgeID")).toString().isEmpty()) { ++edges; }
    }

    const qint64 ms = clock.elapsed();
    std::printf("\n== built in %lld ms ==\n", static_cast<long long>(ms));
    std::printf("objects:       %d\n", objects);
    std::printf("scenes:        %d   <- the progress bar counts THESE\n", scenes);
    std::printf("relationships: %d\n", edges);
    std::printf("\nProject: %s\nWorld:   %s\n",
                qPrintable(projectPath), qPrintable(worldPath));
    std::printf(
        "\n⚠️  To make opening SLOW, put the WORLD on a high-latency mount\n"
        "    (the rig's cache=none,actimeo=1,closetimeo=1 cifs share) and open\n"
        "    the project from the app. Bulk alone on local disk is fast.\n");
    return 0;
}
