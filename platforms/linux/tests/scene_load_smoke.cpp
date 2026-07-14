// scene_load_smoke — headless scene-load check for SP-061 (T-0237, EP-022 AC1/AC2).
//
// Proves the editor's continuous-viewport assembly WITHOUT driving the GUI: it
// exercises the exact code path EditorShell::load() uses —
//   scrivi_open_project (ordered scenes[] + active body)
//     → scrivi_open_scene loop for the other bodies
//     → SceneDocument::build() (one QTextDocument + the per-scene offset map)
// against a multi-scene fixture with DISTINCT bodies, and asserts every body
// landed in the assembled document at the offset the map records.
//
// Fixture setup uses the plain-C ABI directly (create a project, add two more
// scenes, write a unique marker into each scene) so the smoke owns a known,
// multi-scene project.
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-load.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot (see AppSupport).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QPlainTextEdit>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include <scrivi/scrivi.h>

#include "AppSupport.hpp"
#include "SceneDocument.hpp"
#include "ScriviBridge.hpp"

namespace {

// RAII free for the C-ABI strings (mirrors ScriviBridge's guard).
struct CStr {
    const char* p;
    explicit CStr(const char* s) : p(s) {}
    ~CStr() { scrivi_free(p); }
    QString q() const { return p ? QString::fromUtf8(p) : QString(); }
};

constexpr auto kMarkerA = "AAA-scene-one-body";
constexpr auto kMarkerB = "BBB-scene-two-body";
constexpr auto kMarkerC = "CCC-scene-three-body";

} // namespace

int main(int argc, char* argv[])
{
    // QApplication (offscreen platform via the driving script): needed to
    // construct the QPlainTextEdit used in the render assertion below. No window
    // is shown.
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_load_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);

    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Load Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }
    const QString identityID = bridge.identityID();
    const QString personaID  = bridge.personaID();
    const QString displayName = bridge.displayName();

    // --- create the project (one scene) -----------------------------------
    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Load Project"),
                             QStringLiteral("scene-load"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    const auto u8 = [](const QString& s) { return s.toUtf8(); };

    // Helper: create a scene after `afterSceneID` in `chapterID`; returns sceneID
    // (parsed from the {"ok":true,"result":{"sceneID":...}} envelope).
    auto createScene = [&](const QString& chapterID,
                           const QString& afterSceneID) -> QString {
        const CStr env(scrivi_create_scene(
            u8(projectPath).constData(), u8(appSupport).constData(),
            u8(projectID).constData(), u8(chapterID).constData(),
            u8(afterSceneID).constData(), u8(identityID).constData(),
            u8(personaID).constData(), u8(displayName).constData()));
        const QJsonObject root =
            QJsonDocument::fromJson(env.q().toUtf8()).object();
        if (!root.value(QStringLiteral("ok")).toBool()) {
            return QString();
        }
        return root.value(QStringLiteral("result")).toObject()
                   .value(QStringLiteral("sceneID")).toString();
    };

    // Determine the first scene + its chapter from an initial open.
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    if (opened.value(QStringLiteral("mode")).toString() != QStringLiteral("ready")) {
        std::fprintf(stderr, "FAIL: initial open not ready\n");
        return 1;
    }
    QVariantList scenes0 = opened.value(QStringLiteral("scenes")).toList();
    if (scenes0.isEmpty()) {
        std::fprintf(stderr, "FAIL: created project has no scenes\n");
        return 1;
    }
    const QVariantMap firstScene = scenes0.first().toMap();
    const QString chapterID   = firstScene.value(QStringLiteral("chapterID")).toString();
    const QString scene1ID     = firstScene.value(QStringLiteral("sceneID")).toString();

    // --- add two more scenes in the same chapter --------------------------
    const QString scene2ID = createScene(chapterID, scene1ID);
    const QString scene3ID = createScene(chapterID, scene2ID);
    if (scene2ID.isEmpty() || scene3ID.isEmpty()) {
        std::fprintf(stderr, "FAIL: could not create additional scenes\n");
        return 1;
    }

    // --- write a distinct marker body into each scene ---------------------
    // Re-open to get each scene's metadata/content paths for save_scene.
    opened = bridge.openProject(projectPath, appSupport);
    const QVariantList scenes = opened.value(QStringLiteral("scenes")).toList();
    if (scenes.size() != 3) {
        std::fprintf(stderr, "FAIL: expected 3 scenes, got %d\n",
                     static_cast<int>(scenes.size()));
        return 1;
    }

    auto saveBody = [&](const QVariantMap& s, const QString& body) -> bool {
        const CStr env(scrivi_save_scene(
            u8(projectID).constData(), u8(projectPath).constData(),
            u8(appSupport).constData(),
            u8(s.value(QStringLiteral("sceneID")).toString()).constData(),
            u8(s.value(QStringLiteral("metadataPath")).toString()).constData(),
            u8(s.value(QStringLiteral("contentPath")).toString()).constData(),
            u8(body).constData(),
            0, 0, 0.0,
            u8(identityID).constData(), u8(personaID).constData(),
            u8(displayName).constData()));
        const QJsonObject root =
            QJsonDocument::fromJson(env.q().toUtf8()).object();
        return root.value(QStringLiteral("ok")).toBool();
    };

    const char* markers[3] = {kMarkerA, kMarkerB, kMarkerC};
    for (int i = 0; i < 3; ++i) {
        if (!saveBody(scenes.at(i).toMap(), QString::fromLatin1(markers[i]))) {
            std::fprintf(stderr, "FAIL: could not save body for scene %d\n", i);
            return 1;
        }
    }

    // --- replicate EditorShell::load assembly -----------------------------
    opened = bridge.openProject(projectPath, appSupport);
    const QVariantList finalScenes = opened.value(QStringLiteral("scenes")).toList();
    const QVariantMap active = opened.value(QStringLiteral("activeScene")).toMap();
    const QString activeSceneID = active.value(QStringLiteral("sceneID")).toString();
    const QString activeMarkdown = active.value(QStringLiteral("markdown")).toString();

    QList<SceneDocument::Input> inputs;
    for (const QVariant& v : finalScenes) {
        const QVariantMap s = v.toMap();
        const QString sceneID = s.value(QStringLiteral("sceneID")).toString();
        SceneDocument::Input in;
        in.sceneID      = sceneID;
        in.chapterID    = s.value(QStringLiteral("chapterID")).toString();
        in.title        = s.value(QStringLiteral("title")).toString();
        in.chapterTitle = s.value(QStringLiteral("chapterTitle")).toString();
        in.slug         = s.value(QStringLiteral("slug")).toString();
        in.metadataPath = s.value(QStringLiteral("metadataPath")).toString();
        in.contentPath  = s.value(QStringLiteral("contentPath")).toString();
        if (sceneID == activeSceneID) {
            in.markdown = activeMarkdown;
        } else {
            const QVariantMap sc =
                bridge.openScene(projectPath, appSupport, projectID, sceneID);
            in.markdown = sc.value(QStringLiteral("markdown")).toString();
        }
        inputs.append(in);
    }

    SceneDocument doc;
    doc.build(inputs);

    // --- assertions -------------------------------------------------------
    if (doc.segments().size() != 3) {
        std::fprintf(stderr, "FAIL: SceneDocument has %d segments (want 3)\n",
                     static_cast<int>(doc.segments().size()));
        return 1;
    }

    const QString full = doc.document()->toPlainText();
    for (const char* m : markers) {
        if (!full.contains(QString::fromLatin1(m))) {
            std::fprintf(stderr, "FAIL: assembled document is missing marker '%s'\n", m);
            return 1;
        }
    }

    // Each segment's recorded bodyStart must point at its own marker text.
    for (const SceneSegment& seg : doc.segments()) {
        const QString bodyAtStart = full.mid(seg.bodyStart, seg.bodyLength);
        bool matched = false;
        for (const char* m : markers) {
            if (bodyAtStart == QString::fromLatin1(m)) { matched = true; break; }
        }
        if (!matched) {
            std::fprintf(stderr,
                         "FAIL: scene %s body at offset %d ('%s') is not a known marker\n",
                         seg.sceneID.toUtf8().constData(), seg.bodyStart,
                         bodyAtStart.toUtf8().constData());
            return 1;
        }
    }

    // Attaching the assembled document to a real QPlainTextEdit is the surface
    // that actually failed in the app: QPlainTextEdit::setDocument rejects any
    // document lacking a QPlainTextDocumentLayout, leaving the viewport blank
    // ("Document set does not support QPlainTextDocumentLayout"). Reproduce it
    // here — the widget must show the assembled text.
    {
        QPlainTextEdit viewport;
        viewport.setReadOnly(true);
        viewport.setDocument(doc.document());
        const QString shown = viewport.toPlainText();
        if (shown.isEmpty()) {
            std::fprintf(stderr,
                         "FAIL: QPlainTextEdit is blank after setDocument "
                         "(missing QPlainTextDocumentLayout?)\n");
            return 1;
        }
        for (const char* m : markers) {
            if (!shown.contains(QString::fromLatin1(m))) {
                std::fprintf(stderr,
                             "FAIL: viewport text missing marker '%s' after setDocument\n",
                             m);
                return 1;
            }
        }
    }

    std::fprintf(stderr,
                 "OK: 3 scenes assembled into one document; all bodies present at "
                 "their recorded offsets; QPlainTextEdit renders the text.\n");
    std::printf("%s\n", projectID.toUtf8().constData());
    return 0;
}
