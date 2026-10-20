// structured_paste_smoke.cpp — EP-026 S4 (SP-180 / T-0607): structured cut / copy / paste across scenes on Linux.
//
// A REAL `EditorShell` loads a real project; keys go in through `QTest` (the real dispatch); every claim about the
// manuscript is read back FROM DISK (`scrivi_open_project`'s scene list + the scene files), never from the widget.
//
// ✅ What it proves (SP-180 AC0–AC7):
//   • Ctrl+C across scenes holds the structure; the clipboard's text is the pieces joined by a blank line, no heading (Q4);
//   • Ctrl+V rebuilds the scenes — and a chapter, with its title — at the caret, as ONE step Ctrl+Z / Ctrl+Shift+Z replay;
//   • a selection is replaced, the removal its own step (Q2, [I-0295]);
//   • typing not yet saved survives the paste ([I-0294]); the next edit's undo returns to what the paste left ([I-0293]);
//   • anything copied since (here, from another app) expires the structure: the paste is flat (Q1);
//   • the copy buffers hold the structure — Ctrl+1 / Alt+3 across scenes, Ctrl+Shift+N rebuilds, and a slot the Mac wrote;
//   • a history made on the Mac — a `structuredPaste` and an old collapsing `structuredCut` — undoes and redoes here;
//   • a Linux paste's history node carries Apple's payload, key for key.
// ⚠️ NOT covered (live pass only): drag and drop, the rig's clipboard across apps, how it feels.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include "AppSupport.hpp"
#include "BufferService.hpp"
#include "EditorShell.hpp"
#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <cstdio>
#include <memory>

#include <fcntl.h>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

QStringList notices;
void armNoticeCatcher(QObject* owner)
{
    auto* t = new QTimer(owner);
    t->setInterval(20);
    QObject::connect(t, &QTimer::timeout, [] {
        if (auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            notices.append(box->text());
            box->accept();
        }
    });
    t->start();
}

QString compact(const QJsonObject& o) { return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)); }

struct Scene { QString sceneID; QString chapterID; QString chapterTitle; QString text; };

struct Rig {
    ScriviBridge bridge;
    QString projectDir;
    QString appSupport;
    QString projectID;
    std::unique_ptr<EditorShell> shell;
    ManuscriptEditor* editor = nullptr;
    qint64 lastReloadMs = -1;

    bool load()
    {
        shell = std::make_unique<EditorShell>();
        QEventLoop loop;
        bool ok = false;
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) { ok = r; loop.quit(); });
        shell->load(projectDir, appSupport, QStringLiteral("Structured"));
        QTimer::singleShot(60000, &loop, &QEventLoop::quit);
        loop.exec();
        editor = shell->findChild<ManuscriptEditor*>();
        if (!ok || editor == nullptr) {
            return false;
        }
        shell->show();
        shell->activateWindow();
        check(QTest::qWaitForWindowActive(shell.get()), QStringLiteral("the window became active"));
        editor->setFocus();
        QApplication::processEvents();
        return true;
    }
    void close()
    {
        shell->flushHistory();
        shell->saveDirtyScenes();
        shell->releaseProject();
        shell->hide();
        shell.reset();
        editor = nullptr;
    }
    // The manuscript as the core reads it from disk, in reading order.
    QList<Scene> manuscript()
    {
        if (shell != nullptr) {
            shell->saveDirtyScenes();
        }
        QList<Scene> out;
        QStringList ids;
        for (const QVariant& v : bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList()) {
            const QVariantMap s = v.toMap();
            out.append({s.value(QStringLiteral("sceneID")).toString(), s.value(QStringLiteral("chapterID")).toString(),
                        s.value(QStringLiteral("chapterTitle")).toString(), {}});
            ids.append(out.last().sceneID);
        }
        const QHash<QString, QString> texts = bridge.readSceneTexts(projectDir, ids);
        for (Scene& s : out) {
            s.text = texts.value(s.sceneID);
        }
        return out;
    }
    QStringList texts()
    {
        QStringList t;
        for (const Scene& s : manuscript()) {
            t.append(s.text);
        }
        return t;
    }
    void caretAt(const QString& marker, bool before = false)
    {
        const QTextCursor found = editor->document()->find(marker);
        QTextCursor c = editor->textCursor();
        c.setPosition(before ? found.selectionStart() : found.selectionEnd());
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void select(const QString& from, const QString& to)
    {
        const QTextCursor a = editor->document()->find(from);
        const QTextCursor b = editor->document()->find(to);
        QTextCursor c = editor->textCursor();
        c.setPosition(a.selectionStart());
        c.setPosition(b.selectionEnd(), QTextCursor::KeepAnchor);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void type(const QString& text) { QTest::keyClicks(editor, text); QApplication::processEvents(); }
    void control(Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier | extra);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier | extra);
        settle();
    }
    // `act` reloads the manuscript (a structured paste, or an undo/redo across one): wait for the reload, timed.
    bool reloading(const std::function<void()>& act)
    {
        QEventLoop loop;
        bool ok = false;
        bool done = false;
        QElapsedTimer t;
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) {
            ok = r;
            done = true;
            lastReloadMs = t.elapsed();
            loop.quit();
        });
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        t.start();
        act();
        if (!done) {   // ⚠️ the reload can finish inside `act`'s own wait — a quit before exec() is lost
            loop.exec();
        }
        settle();
        return ok;
    }
    void copy() { control(Qt::Key_C); }
    bool paste() { return reloading([this] { control(Qt::Key_V); }); }
    void undo() { control(Qt::Key_Z); }
    void redo() { control(Qt::Key_Z, Qt::ShiftModifier); }
    bool undoStructural() { return reloading([this] { undo(); }); }
    bool redoStructural() { return reloading([this] { redo(); }); }
    void bufferCopy(int n) { QTest::keyClick(editor, Qt::Key(Qt::Key_0 + n), Qt::ControlModifier); settle(); }
    void bufferCut(int n) { QTest::keyClick(editor, Qt::Key(Qt::Key_0 + n), Qt::AltModifier); settle(); }
    bool bufferPaste(int n)
    {
        static const Qt::Key shifted[] = {Qt::Key_Exclam, Qt::Key_At, Qt::Key_NumberSign, Qt::Key_Dollar, Qt::Key_Percent,
                                          Qt::Key_AsciiCircum, Qt::Key_Ampersand, Qt::Key_Asterisk, Qt::Key_ParenLeft};
        static const char* marks = "!@#$%^&*(";
        return reloading([this, n] {
            const QString t(QChar::fromLatin1(marks[n - 1]));
            QTest::sendKeyEvent(QTest::Press, editor, shifted[n - 1], t, Qt::ControlModifier | Qt::ShiftModifier);
            QTest::sendKeyEvent(QTest::Release, editor, shifted[n - 1], t, Qt::ControlModifier | Qt::ShiftModifier);
        });
    }
    static void settle() { QTest::qWait(60); }
};

const QString A = QStringLiteral("Alpha one. Alpha two.");
const QString B = QStringLiteral("Bravo one. Bravo two.");
const QString C = QStringLiteral("Charlie one. Charlie two.");
const QString D = QStringLiteral("Delta one. Delta two.");
const QString kCasket = QStringLiteral("The Black Casket");

QString shown(const QStringList& l) { return QStringLiteral("[\"%1\"]").arg(l.join(QStringLiteral("\", \""))); }

// The structural records in the project's history log, each payload parsed.
QList<QJsonObject> structuralPayloads(const QString& projectDir)
{
    QList<QJsonObject> out;
    const QDir dir(projectDir + QStringLiteral("/history"));
    for (const QString& name : dir.entryList({QStringLiteral("log-*.jsonl")}, QDir::Files, QDir::Name)) {
        QFile f(dir.filePath(name));
        if (!f.open(QIODevice::ReadOnly)) {
            continue;
        }
        for (const QByteArray& line : f.readAll().split('\n')) {
            const QJsonObject rec = QJsonDocument::fromJson(line).object();
            if (rec.value(QStringLiteral("kind")).toString() == QStringLiteral("structural")) {
                out.append(QJsonDocument::fromJson(rec.value(QStringLiteral("structuralPayload")).toString().toUtf8()).object());
            }
        }
    }
    return out;
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: structured_paste_smoke <projectDir>\n");
        return 2;
    }
    armNoticeCatcher(&app);

    Rig rig;
    rig.projectDir = QString::fromUtf8(argv[1]);
    rig.appSupport = scrivi::linux_app::appSupportRoot();
    rig.bridge.bootstrap(QStringLiteral("Scrivi"), rig.appSupport);
    rig.projectID = rig.bridge.createProject(rig.projectDir, rig.appSupport, QStringLiteral("Structured"),
                                             QStringLiteral("structured")).value(QStringLiteral("projectID")).toString();
    check(!rig.projectID.isEmpty(), QStringLiteral("project created"));

    // Chapter 1 [A, B] · chapter 2 "The Black Casket" [C] · chapter 3 [D] — seeded on disk before the first open.
    {
        auto scenes = [&] { return rig.bridge.openProject(rig.projectDir, rig.appSupport).value(QStringLiteral("scenes")).toList(); };
        const QVariantMap first = scenes().first().toMap();
        rig.bridge.createScene(rig.projectDir, rig.appSupport, rig.projectID, first.value(QStringLiteral("chapterID")).toString(),
                               first.value(QStringLiteral("sceneID")).toString());
        rig.bridge.createChapter(rig.projectDir, rig.appSupport, rig.projectID);
        rig.bridge.createChapter(rig.projectDir, rig.appSupport, rig.projectID);
        const QVariantList all = scenes();
        check(all.size() == 4, QStringLiteral("four scenes seeded (got %1)").arg(all.size()));
        const QStringList seeds{A, B, C, D};
        for (int i = 0; i < all.size() && i < 4; ++i) {
            const QVariantMap s = all.at(i).toMap();
            rig.bridge.saveScene(rig.projectID, rig.projectDir, rig.appSupport, s.value(QStringLiteral("sceneID")).toString(),
                                 s.value(QStringLiteral("metadataPath")).toString(),
                                 s.value(QStringLiteral("contentPath")).toString(), seeds.at(i), 0, 0, 0.0);
        }
        rig.bridge.renameChapter(rig.projectDir, all.at(2).toMap().value(QStringLiteral("chapterMetadataPath")).toString(),
                                 kCasket);
        rig.bridge.closeProject(rig.projectDir);
    }
    const QStringList base{A, B, C, D};

    check(rig.load(), QStringLiteral("project loaded"));
    check(rig.texts() == base, QStringLiteral("the seeds: %1").arg(shown(rig.texts())));

    // ── 1. Ctrl+C across a scene break; Ctrl+V rebuilds it; ONE step, replayed both ways ──────────────────────────
    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.copy();
    check(QApplication::clipboard()->text() == QStringLiteral("Alpha two.\n\nBravo one."),
          QStringLiteral("Q4: the clipboard's text is the pieces joined by a blank line (got \"%1\")")
              .arg(QApplication::clipboard()->text()));
    // ✅ [I-0307]: a mouse selection afterwards fills the PRIMARY selection (X11 / Wayland) — with the flat text only.
    // ⛔ It used to build the structured fragment there: a ScriviCore extract on the GUI thread (the rig's freeze "after
    // select across scene") that also replaced the held copy, so the next Ctrl+V was flat.
    rig.select(QStringLiteral("Alpha one."), QStringLiteral("Bravo two."));
    {
        std::unique_ptr<QMimeData> primary(rig.editor->primarySelectionMime());
        check(primary != nullptr && primary->text().startsWith(QStringLiteral("Alpha one."))
                  && primary->text().endsWith(QStringLiteral("Bravo two.")),
              QStringLiteral("[I-0307] the primary selection holds the writer's text (got \"%1\")")
                  .arg(primary != nullptr ? primary->text() : QString()));
        check(primary != nullptr && !primary->hasFormat(QStringLiteral("application/x-scrivi-fragment-token")),
              QStringLiteral("[I-0307] … and no structured copy (no fragment token)"));
    }
    rig.caretAt(QStringLiteral("Charlie one."));
    check(rig.paste(), QStringLiteral("Ctrl+V reloaded the manuscript ([I-0307]: the mouse selection left the copy alone)"));
    const qint64 pasteMs = rig.lastReloadMs;
    const QStringList pasted{A, B, QStringLiteral("Charlie one.Alpha two."), QStringLiteral("Bravo one. Charlie two."), D};
    QList<Scene> m = rig.manuscript();
    check(rig.texts() == pasted, QStringLiteral("Ctrl+V rebuilt the scene break at the caret: %1").arg(shown(rig.texts())));
    check(m.size() == 5 && m.at(3).chapterID == m.at(2).chapterID,
          QStringLiteral("the new scene is in the target's chapter"));
    const int caretScene = rig.editor->document()->find(QStringLiteral("Charlie one.Alpha two.")).isNull() ? -1 : 0;
    check(caretScene == 0, QStringLiteral("the editor shows the pasted text after the reload"));

    check(rig.undoStructural(), QStringLiteral("Ctrl+Z over the paste reloaded"));
    check(rig.texts() == base, QStringLiteral("ONE Ctrl+Z took the whole paste back: %1").arg(shown(rig.texts())));
    check(rig.redoStructural(), QStringLiteral("Ctrl+Shift+Z over the paste reloaded"));
    check(rig.texts() == pasted, QStringLiteral("Ctrl+Shift+Z pasted it again: %1").arg(shown(rig.texts())));
    check(rig.undoStructural(), QStringLiteral("Ctrl+Z again reloaded"));
    check(rig.texts() == base, QStringLiteral("and Ctrl+Z took it back again: %1").arg(shown(rig.texts())));

    // ── 2. Across a CHAPTER: the chapter comes back with its title; unsaved typing survives ([I-0294]); the next edit's
    //       undo returns to what the paste left ([I-0293]) ────────────────────────────────────────────────────────────
    rig.select(QStringLiteral("Bravo two."), QStringLiteral("Charlie one."));
    rig.copy();
    const QString clip = QApplication::clipboard()->text();
    check(clip == QStringLiteral("Bravo two.\n\nCharlie one."),
          QStringLiteral("Q4: no chapter heading in the clipboard's text (got \"%1\")").arg(clip));
    rig.caretAt(QStringLiteral("Delta one."));
    rig.type(QStringLiteral(" Xray"));   // ⚠️ not saved yet — the paste follows at once
    check(rig.paste(), QStringLiteral("the cross-chapter paste reloaded"));
    m = rig.manuscript();
    const QStringList chapterPasted{A, B, C, QStringLiteral("Delta one. XrayBravo two."), QStringLiteral("Charlie one. Delta two.")};
    check(rig.texts() == chapterPasted,
          QStringLiteral("[I-0294] the typing survived, and the chapter piece rebuilt: %1").arg(shown(rig.texts())));
    check(m.size() == 5 && m.at(4).chapterID != m.at(3).chapterID && m.at(4).chapterTitle == kCasket,
          QStringLiteral("a NEW chapter titled \"%1\" (got \"%2\")").arg(kCasket, m.size() == 5 ? m.at(4).chapterTitle : QString()));

    rig.caretAt(QStringLiteral("XrayBravo two."));
    rig.type(QStringLiteral(" Yes"));
    rig.undo();
    check(rig.texts().value(3) == QStringLiteral("Delta one. XrayBravo two."),
          QStringLiteral("[I-0293] the next edit's undo returns to what the paste left (got \"%1\")").arg(rig.texts().value(3)));
    check(rig.undoStructural(), QStringLiteral("Ctrl+Z over the cross-chapter paste reloaded"));
    m = rig.manuscript();
    check(rig.texts() == QStringList({A, B, C, QStringLiteral("Delta one. Xray Delta two.")}) && m.size() == 4
              && m.at(3).chapterID != m.at(2).chapterID,
          QStringLiteral("[I-0293] the paste's own undo was accepted — its chapter gone: %1").arg(shown(rig.texts())));
    rig.undo();
    check(rig.texts() == base, QStringLiteral("and the typing before it is its own step: %1").arg(shown(rig.texts())));

    // ── 3. Q2 ([I-0295]): a selection is REPLACED — the removal its own step ────────────────────────────────────────
    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.copy();
    rig.select(QStringLiteral("Delta"), QStringLiteral("Delta"));
    check(rig.paste(), QStringLiteral("the paste over a selection reloaded"));
    check(rig.texts() == QStringList({A, B, C, QStringLiteral("Alpha two."), QStringLiteral("Bravo one. one. Delta two.")}),
          QStringLiteral("the selected word was replaced: %1").arg(shown(rig.texts())));
    check(rig.undoStructural(), QStringLiteral("Ctrl+Z over it reloaded"));
    check(rig.texts() == QStringList({A, B, C, QStringLiteral(" one. Delta two.")}),
          QStringLiteral("Ctrl+Z took back the paste, not the removal: %1").arg(shown(rig.texts())));
    rig.undo();
    check(rig.texts() == base, QStringLiteral("and Ctrl+Z again restored the word: %1").arg(shown(rig.texts())));

    // ── 4. Q1: anything copied since — here from another app — expires the structure; the paste is flat ────────────
    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.copy();
    QApplication::clipboard()->setText(QStringLiteral("Outside words"));
    rig.caretAt(QStringLiteral("Delta two."));
    rig.control(Qt::Key_V);
    check(rig.texts() == QStringList({A, B, C, D + QStringLiteral("Outside words")}),
          QStringLiteral("an expired structure pastes flat: %1").arg(shown(rig.texts())));
    rig.undo();
    check(rig.texts() == base, QStringLiteral("(undone): %1").arg(shown(rig.texts())));
    // …and so does a copy in ANOTHER Scrivi window: its clipboard data carries that window's token, not this editor's.
    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.copy();
    {
        auto* other = new QMimeData;
        other->setText(QStringLiteral("Other window"));
        other->setData(QStringLiteral("application/x-scrivi-fragment-token"), QByteArrayLiteral("another-editor:1"));
        QApplication::clipboard()->setMimeData(other);
    }
    rig.caretAt(QStringLiteral("Delta two."));
    rig.control(Qt::Key_V);
    check(rig.texts() == QStringList({A, B, C, D + QStringLiteral("Other window")}),
          QStringLiteral("another window's copy pastes flat, never this editor's held structure: %1").arg(shown(rig.texts())));
    rig.undo();
    check(rig.texts() == base, QStringLiteral("(undone): %1").arg(shown(rig.texts())));

    // ── 4b. [T-0591]: pasted INSIDE bold, the pasted text keeps its own formatting — the span closes before it and
    //        re-opens after it (Apple: `balancePastePieces`) ────────────────────────────────────────────────────────────
    rig.select(QStringLiteral("Delta"), QStringLiteral("Delta"));
    rig.editor->applyFormat(ManuscriptEditor::Format::Bold);
    rig.settle();
    check(rig.texts().value(3) == QStringLiteral("**Delta** one. Delta two."),
          QStringLiteral("(precondition: a bold word, got \"%1\")").arg(rig.texts().value(3)));
    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.copy();
    rig.caretAt(QStringLiteral("**Del"));
    check(rig.paste(), QStringLiteral("the paste inside bold reloaded"));
    check(rig.texts() == QStringList({A, B, C, QStringLiteral("**Del**Alpha two."), QStringLiteral("Bravo one.**ta** one. Delta two.")}),
          QStringLiteral("the bold closed before the paste and re-opened after it: %1").arg(shown(rig.texts())));
    check(rig.undoStructural(), QStringLiteral("(undone, reloaded)"));
    rig.undo();
    check(rig.texts() == base, QStringLiteral("(the bold undone too): %1").arg(shown(rig.texts())));

    // ── 5. The copy buffers hold the structure ──────────────────────────────────────────────────────────────────
    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.bufferCopy(1);
    check(ScriviBridge::buffersGet(rig.projectDir, QStringLiteral("1")).contains(QStringLiteral("fragment")),
          QStringLiteral("Ctrl+1 across scenes stored the fragment"));
    check(rig.texts() == base, QStringLiteral("a copy into a slot changes nothing"));
    rig.caretAt(QStringLiteral("Charlie two."));
    check(rig.bufferPaste(1), QStringLiteral("Ctrl+Shift+1 of a structured slot reloaded"));
    check(rig.texts() == QStringList({A, B, C + QStringLiteral("Alpha two."), QStringLiteral("Bravo one."), D}),
          QStringLiteral("Ctrl+Shift+1 rebuilt the scene break: %1").arg(shown(rig.texts())));
    check(rig.undoStructural(), QStringLiteral("(undone, reloaded)"));
    check(rig.texts() == base, QStringLiteral("(undone): %1").arg(shown(rig.texts())));

    // A slot the MAC wrote (`buffers.json` is shared): its fragment rebuilds here too ([SP-179] Q1's notice is gone).
    const QJsonObject macFragment{
        {QStringLiteral("schema"), QStringLiteral("scrivi.fragment.v1")},
        {QStringLiteral("pieces"), QJsonArray{
             QJsonObject{{QStringLiteral("opensWith"), QStringLiteral("none")}, {QStringLiteral("text"), QStringLiteral("Mac one")},
                         {QStringLiteral("partial"), QStringLiteral("head")}},
             QJsonObject{{QStringLiteral("opensWith"), QStringLiteral("scene")}, {QStringLiteral("text"), QStringLiteral("Mac two")},
                         {QStringLiteral("partial"), QStringLiteral("tail")}}}},
        {QStringLiteral("plainText"), QStringLiteral("Mac one\n\nMac two")}};
    ScriviBridge::buffersLoad(rig.projectDir, QStringLiteral("2"), QStringLiteral("Mac one\n\nMac two"), compact(macFragment));
    rig.shell->buffers()->refresh();
    rig.caretAt(QStringLiteral("Delta two."));
    check(rig.bufferPaste(2), QStringLiteral("Ctrl+Shift+2 of the Mac's slot reloaded"));
    check(rig.texts() == QStringList({A, B, C, D + QStringLiteral("Mac one"), QStringLiteral("Mac two")}),
          QStringLiteral("the Mac's structured slot rebuilt its scene break: %1").arg(shown(rig.texts())));
    check(rig.undoStructural(), QStringLiteral("(undone, reloaded)"));
    check(rig.texts() == base, QStringLiteral("(undone): %1").arg(shown(rig.texts())));

    rig.select(QStringLiteral("Alpha two."), QStringLiteral("Bravo one."));
    rig.bufferCut(3);
    check(ScriviBridge::buffersGet(rig.projectDir, QStringLiteral("3")).contains(QStringLiteral("fragment")),
          QStringLiteral("Alt+3 across scenes stored the fragment"));
    check(rig.texts() == QStringList({QStringLiteral("Alpha one. "), QStringLiteral(" Bravo two."), C, D}),
          QStringLiteral("Alt+3 across scenes kept the scenes ([I-0270]): %1").arg(shown(rig.texts())));
    rig.undo();
    check(rig.texts() == base, QStringLiteral("(undone in one step): %1").arg(shown(rig.texts())));
    check(notices.isEmpty(), QStringLiteral("no notices so far (got: %1)").arg(notices.join(QStringLiteral(" | "))));

    // ── 6. The Linux paste's history node carries Apple's payload, key for key ─────────────────────────────────────
    {
        const QList<QJsonObject> payloads = structuralPayloads(rig.projectDir);
        check(!payloads.isEmpty(), QStringLiteral("the log holds structural nodes"));
        const QStringList keys{QStringLiteral("op"), QStringLiteral("fragmentJSON"), QStringLiteral("caretSceneID"),
                               QStringLiteral("caretByte"), QStringLiteral("createdSceneIDs"), QStringLiteral("createdChapterIDs"),
                               QStringLiteral("removedSceneIDs"), QStringLiteral("removedChapterIDs")};
        for (const QJsonObject& p : payloads) {
            QStringList have = p.keys();
            have.sort();
            QStringList want = keys;
            want.sort();
            check(have == want && p.value(QStringLiteral("op")).toString() == QStringLiteral("structuredPaste"),
                  QStringLiteral("a payload with Apple's keys (got %1)").arg(have.join(QLatin1Char(','))));
        }
    }

    // ── 7. A history made on the MAC: a `structuredPaste`, then an old collapsing `structuredCut` ───────────────────
    // An edit KEPT on Linux first, so the Mac's paste sits on real work from an earlier session — an undo across it
    // crosses a session boundary ([I-0298]), as in the rig's step 36.
    rig.caretAt(QStringLiteral("Delta two."));
    rig.type(QStringLiteral(" Kept"));
    const QString D2 = D + QStringLiteral(" Kept");
    const QStringList kept{A, B, C, D2};
    rig.close();
    {
        // The Mac pastes [none "Mac one", scene "Mac two"] at "Delta one.|", the way `pasteStructuredFragment` records it.
        QList<Scene> s = rig.manuscript();
        ScriviBridge::historyOpen(rig.projectDir);
        const QVariantMap r = rig.bridge.fragmentPaste(rig.projectDir, rig.appSupport, rig.projectID, compact(macFragment),
                                                       s.at(3).sceneID, 10, rig.bridge.displayName());
        const QStringList created = r.value(QStringLiteral("createdSceneIDs")).toStringList();
        const QJsonObject payload{{QStringLiteral("op"), QStringLiteral("structuredPaste")},
                                  {QStringLiteral("fragmentJSON"), compact(macFragment)},
                                  {QStringLiteral("caretSceneID"), s.at(3).sceneID},
                                  {QStringLiteral("caretByte"), 10},
                                  {QStringLiteral("createdSceneIDs"), QJsonArray::fromStringList(created)},
                                  {QStringLiteral("createdChapterIDs"), QJsonArray()},
                                  {QStringLiteral("removedSceneIDs"), QJsonArray()},
                                  {QStringLiteral("removedChapterIDs"), QJsonArray()}};
        // With the texts on both sides of it, as the Mac records them since [I-0293].
        QJsonObject after;
        for (const auto& [id, text] : rig.bridge.readSceneTexts(rig.projectDir, QStringList{s.at(3).sceneID} + created).asKeyValueRange()) {
            after.insert(id, text);
        }
        ScriviBridge::captureRecordBarrier(rig.projectDir, compact(QJsonObject{
            {QStringLiteral("barrierKind"), QStringLiteral("structuredPaste")}, {QStringLiteral("note"), QStringLiteral("mac")},
            {QStringLiteral("structuralPayload"), payload}, {QStringLiteral("sceneTexts"), after},
            {QStringLiteral("sceneTextsBefore"), QJsonObject{{s.at(3).sceneID, D2}}}}));
        ScriviBridge::historyClose(rig.projectDir);
        rig.bridge.closeProject(rig.projectDir);
    }
    const QStringList macPasted{A, B, C, QStringLiteral("Delta one.Mac one"), QStringLiteral("Mac two Delta two. Kept")};
    check(rig.texts() == macPasted, QStringLiteral("(the Mac's paste, on disk): %1").arg(shown(rig.texts())));
    check(rig.load(), QStringLiteral("reopened after the Mac's paste"));
    notices.clear();
    check(rig.undoStructural(), QStringLiteral("Ctrl+Z over the Mac's paste reloaded"));
    // [I-0298]: the undo crossed into an earlier session — the warning, as for any undo.
    check(notices.contains(QStringLiteral("Undoing changes from a previous session")),
          QStringLiteral("[I-0298] the previous-session warning after a structural undo (notices: %1)")
              .arg(notices.join(QStringLiteral(" | "))));
    check(rig.texts() == kept, QStringLiteral("the Mac's paste undone on Linux: %1").arg(shown(rig.texts())));
    check(rig.redoStructural(), QStringLiteral("Ctrl+Shift+Z reloaded"));
    check(rig.texts() == macPasted, QStringLiteral("and redone on Linux: %1").arg(shown(rig.texts())));
    check(rig.undoStructural(), QStringLiteral("(undone again)"));
    check(rig.texts() == kept, QStringLiteral("(undone again): %1").arg(shown(rig.texts())));

    rig.close();
    {
        // The Mac's OLD cross-scene cut (EP-029 §4.3, before [I-0270]): "Alpha two." … "Bravo one." cut and the two scenes
        // COLLAPSED into one; recorded as `structuredCut` anchored at the fold.
        QList<Scene> s = rig.manuscript();
        ScriviBridge::historyOpen(rig.projectDir);
        const QJsonArray spans{
            QJsonObject{{QStringLiteral("sceneID"), s.at(0).sceneID}, {QStringLiteral("startByte"), 11}, {QStringLiteral("endByte"), 21}},
            QJsonObject{{QStringLiteral("sceneID"), s.at(1).sceneID}, {QStringLiteral("startByte"), 0}, {QStringLiteral("endByte"), 10}}};
        const QVariantMap cut = rig.bridge.fragmentCut(rig.projectDir, compact(QJsonObject{{QStringLiteral("spans"), spans}}));
        const QJsonObject payload{
            {QStringLiteral("op"), QStringLiteral("structuredCut")},
            {QStringLiteral("fragmentJSON"), compact(QJsonObject::fromVariantMap(cut.value(QStringLiteral("fragment")).toMap()))},
            {QStringLiteral("caretSceneID"), cut.value(QStringLiteral("survivingSceneID")).toString()},
            {QStringLiteral("caretByte"), 11},
            {QStringLiteral("createdSceneIDs"), QJsonArray()}, {QStringLiteral("createdChapterIDs"), QJsonArray()},
            {QStringLiteral("removedSceneIDs"), QJsonArray::fromStringList(cut.value(QStringLiteral("removedSceneIDs")).toStringList())},
            {QStringLiteral("removedChapterIDs"), QJsonArray()}};
        // ⚠️ With the survivor's texts, so the reopen is clean: this section is about REPLAYING a `structuredCut` payload.
        // (A log from before [I-0293] carried none; its reopen raises an outside-change barrier, as any stale head does.)
        const QString survivor = cut.value(QStringLiteral("survivingSceneID")).toString();
        ScriviBridge::captureRecordBarrier(rig.projectDir, compact(QJsonObject{
            {QStringLiteral("barrierKind"), QStringLiteral("structuredCut")}, {QStringLiteral("note"), QStringLiteral("mac")},
            {QStringLiteral("structuralPayload"), payload},
            {QStringLiteral("sceneTexts"), QJsonObject{{survivor, QStringLiteral("Alpha one.  Bravo two.")}}},
            {QStringLiteral("sceneTextsBefore"), QJsonObject{{survivor, A}}}}));
        ScriviBridge::historyClose(rig.projectDir);
        rig.bridge.closeProject(rig.projectDir);
    }
    const QStringList collapsed{QStringLiteral("Alpha one.  Bravo two."), C, D2};
    check(rig.texts() == collapsed, QStringLiteral("(the Mac's old cut, on disk): %1").arg(shown(rig.texts())));
    check(rig.load(), QStringLiteral("reopened after the Mac's cut"));
    check(rig.undoStructural(), QStringLiteral("Ctrl+Z over the Mac's cut reloaded"));
    check(rig.texts() == kept, QStringLiteral("the Mac's collapsing cut undone on Linux — scenes and text back: %1").arg(shown(rig.texts())));
    check(rig.redoStructural(), QStringLiteral("Ctrl+Shift+Z reloaded"));
    check(rig.texts() == collapsed, QStringLiteral("and redone on Linux: %1").arg(shown(rig.texts())));
    for (const QString& n : std::as_const(notices)) {
        check(!n.contains(QStringLiteral("outside Scrivi")),
              QStringLiteral("no outside-change barrier after the Mac's own writes (got: %1)").arg(n));
    }

    std::printf("structured paste: paste + reload %lld ms (5 scenes)\n", static_cast<long long>(pasteMs));
    rig.close();

    // ── 8. MEASURED, opt-in (SCRIVI_MEASURE_LARGE=1; CI does not pay for it): every structured paste and every structural
    //       undo reloads the manuscript — timed on 1,186 scenes (~1.8 MB), the size of the writer's real manuscript ──────
    if (qEnvironmentVariableIsSet("SCRIVI_MEASURE_LARGE")) {
        Rig big;
        big.projectDir = rig.projectDir + QStringLiteral("-large");
        big.appSupport = rig.appSupport;
        big.bridge.bootstrap(QStringLiteral("Scrivi"), big.appSupport);
        big.projectID = big.bridge.createProject(big.projectDir, big.appSupport, QStringLiteral("Large"), QStringLiteral("large"))
                            .value(QStringLiteral("projectID")).toString();
        QElapsedTimer t;
        t.start();
        const QString para = QStringLiteral("It was the best of times %1, it was the worst of times; Mr Defarge said nothing %2. ");
        auto body = [&](int i) {
            QString b;
            for (int k = 0; b.size() < 1500; ++k) {
                b += para.arg(i).arg(k);
            }
            return b;
        };
        QVariantMap first = big.bridge.openProject(big.projectDir, big.appSupport).value(QStringLiteral("scenes")).toList().first().toMap();
        QString chapterID = first.value(QStringLiteral("chapterID")).toString();
        QString after = first.value(QStringLiteral("sceneID")).toString();
        big.bridge.saveScene(big.projectID, big.projectDir, big.appSupport, after, first.value(QStringLiteral("metadataPath")).toString(),
                             first.value(QStringLiteral("contentPath")).toString(), body(0), 0, 0, 0.0);
        for (int i = 1; i < 1186; ++i) {
            QVariantMap s;
            if (i % 20 == 0) {
                const QVariantMap c = big.bridge.createChapter(big.projectDir, big.appSupport, big.projectID);
                chapterID = c.value(QStringLiteral("chapterID")).toString();
                s = {{QStringLiteral("sceneID"), c.value(QStringLiteral("firstSceneID"))},
                     {QStringLiteral("metadataPath"), c.value(QStringLiteral("firstSceneMetadataPath"))},
                     {QStringLiteral("contentPath"), c.value(QStringLiteral("firstSceneContentPath"))}};
            } else {
                s = big.bridge.createScene(big.projectDir, big.appSupport, big.projectID, chapterID, after);
            }
            after = s.value(QStringLiteral("sceneID")).toString();
            big.bridge.saveScene(big.projectID, big.projectDir, big.appSupport, after, s.value(QStringLiteral("metadataPath")).toString(),
                                 s.value(QStringLiteral("contentPath")).toString(), body(i), 0, 0, 0.0);
        }
        big.bridge.closeProject(big.projectDir);
        std::printf("measure: built %d scenes in %lld ms\n", 1186, static_cast<long long>(t.elapsed()));

        t.restart();
        check(big.load(), QStringLiteral("the large project loaded"));
        std::printf("measure: first open %lld ms\n", static_cast<long long>(t.elapsed()));
        check(big.manuscript().size() == 1186, QStringLiteral("1,186 scenes"));
        big.select(QStringLiteral("best of times 600, it was"), QStringLiteral("best of times 601, it was"));
        big.copy();
        big.caretAt(QStringLiteral("best of times 900, it was"));
        // Markers for a syscall trace ([I-0297]): a filesystem probe of a path that never exists, so `strace` can split
        // the calls one structured paste makes — and on which thread.
        (void)::open("/scrivi-mark/paste-begin", O_RDONLY);   // fails: ENOENT
        check(big.paste(), QStringLiteral("the large paste reloaded"));
        (void)::open("/scrivi-mark/paste-end", O_RDONLY);   // fails: ENOENT
        std::printf("measure: structured paste + reload %lld ms\n", static_cast<long long>(big.lastReloadMs));
        check(big.manuscript().size() == 1187, QStringLiteral("one scene more after the paste"));
        (void)::open("/scrivi-mark/undo-begin", O_RDONLY);   // fails: ENOENT
        check(big.undoStructural(), QStringLiteral("the large undo reloaded"));
        (void)::open("/scrivi-mark/undo-end", O_RDONLY);   // fails: ENOENT
        std::printf("measure: structural undo + reload %lld ms\n", static_cast<long long>(big.lastReloadMs));
        check(big.manuscript().size() == 1186, QStringLiteral("back to 1,186 after the undo"));
        big.close();
    }
    if (failures == 0) {
        std::printf("structured-paste-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
