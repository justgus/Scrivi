// escape_smoke — EP-049 (SP-160, T-0586): Linux writes Apple's manuscript format.
//
// Drives the REAL ManuscriptEditor (over a real SceneDocument, wired exactly as EditorShell wires it)
// through the SHARED corpus `ScriviCore/tests/fixtures/manuscript_format_corpus.json` — the same cases
// Apple's interop suite runs through ManuscriptNSTextView (EP-049 AC8). A failure here means the two
// platforms would write DIFFERENT bytes for the same gesture.
// Plus Linux-only gestures the corpus cannot express on both platforms: input-method commits (M2)
// and Ctrl+Delete's word delete (M3).

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QInputMethodEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMimeData>
#include <QRegularExpression>
#include <QTextCursor>

#include <cstdio>

#include "ManuscriptEditor.hpp"
#include "ManuscriptEscapes.hpp"
#include "SceneDocument.hpp"

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", qPrintable(what));
        ++failures;
    }
}

QString visible(QString s)
{
    s.replace(QLatin1Char('\n'), QStringLiteral("⏎"));
    return s;
}

// One scene whose body is `body`, followed by a second chapter's scene (so the body has real
// neighbours and a real boundary after it, as in the app).
struct Harness {
    SceneDocument doc;
    ManuscriptEditor editor;
    explicit Harness(const QString& body)
    {
        QList<SceneDocument::Input> in;
        in.push_back({"s1", "c1", "Scene 1", "One", "s1", "", "", "", body});
        in.push_back({"s2", "c2", "Scene 2", "Two", "s2", "", "", "", "Tail"});
        doc.build(in);
        editor.setDocument(doc.document());
        editor.setSceneDocument(&doc);
        QObject::connect(editor.document(), &QTextDocument::contentsChange,
                         [this](int pos, int removed, int added) {
                             doc.applyContentsChange(pos, removed, added);
                         });
    }
    int base() const { return doc.segments().at(0).bodyStart; }
    QString body() const { return doc.bodyText(0); }
    void select(int start, int end)
    {
        QTextCursor c = editor.textCursor();
        c.setPosition(base() + start);
        c.setPosition(base() + end, QTextCursor::KeepAnchor);
        editor.setTextCursor(c);
    }
    void caret(int at) { select(at, at); }
    void key(int k, Qt::KeyboardModifiers m, const QString& text)
    {
        QKeyEvent e(QEvent::KeyPress, k, m, text);
        QApplication::sendEvent(&editor, &e);
    }
    void type(const QString& text)
    {
        for (const QChar c : text) {
            const int k = c.isLetter() ? c.toUpper().unicode() : int(Qt::Key_unknown);
            key(k, Qt::NoModifier, QString(c));
        }
    }
};

void runCorpus(const QString& path)
{
    QFile f(path);
    check(f.open(QIODevice::ReadOnly), "open corpus " + path);
    const QJsonArray cases = QJsonDocument::fromJson(f.readAll()).object().value("cases").toArray();
    check(cases.size() >= 18, "corpus has its cases");
    for (const QJsonValue& v : cases) {
        const QJsonObject c = v.toObject();
        const QString name = c.value("name").toString();
        Harness h(c.value("before").toString());
        if (c.contains("selection")) {
            const QJsonArray s = c.value("selection").toArray();
            h.select(s.at(0).toInt(), s.at(1).toInt());
        } else {
            h.caret(c.value("caret").toInt());
        }
        const QString g = c.value("gesture").toString();
        const QString text = c.value("text").toString();
        if (g == "type") {
            h.type(text);
        } else if (g == "return") {
            h.key(Qt::Key_Return, Qt::NoModifier, "\r");
        } else if (g == "enter") {
            h.key(Qt::Key_Enter, Qt::KeypadModifier, "\r");
        } else if (g == "shiftReturn") {
            h.key(Qt::Key_Return, Qt::ShiftModifier, "\r");
        } else if (g == "altReturn") {
            h.key(Qt::Key_Return, Qt::AltModifier, "\r");
        } else if (g == "backspace") {
            h.key(Qt::Key_Backspace, Qt::NoModifier, "\b");
        } else if (g == "delete") {
            h.key(Qt::Key_Delete, Qt::NoModifier, QString(QChar(0x7f)));
        } else if (g == "paste") {
            auto* m = new QMimeData;
            m->setText(text);
            QApplication::clipboard()->setMimeData(m);
            h.editor.paste();
        } else if (g == "copy") {
            h.editor.copy();
            check(QApplication::clipboard()->text() == c.value("clipboard").toString(),
                  name + ": clipboard = " + visible(QApplication::clipboard()->text()));
        } else if (g == "copyThenPasteAtEnd") {
            h.editor.copy();
            h.caret(h.body().size());
            h.editor.paste();
        } else {
            check(false, name + ": unknown gesture " + g);
        }
        const QString want = c.value("after").toString();
        check(h.body() == want, name + ": stored " + visible(h.body()) + ", expected " + visible(want));
    }
}

void runLinuxOnly()
{
    // M2: a composed '*' commits through inputMethodEvent, never keyPressEvent — escaped all the same.
    {
        Harness h("");
        h.caret(0);
        QInputMethodEvent e;
        e.setCommitString("*");
        QApplication::sendEvent(&h.editor, &e);
        check(h.body() == "\\*", "M2: an input-method commit is escaped (stored " + visible(h.body()) + ")");
    }
    // M3: Ctrl+Delete (word) never leaves half an escape pair, from anywhere in the line.
    for (int at = 0; at <= 7; ++at) {
        Harness h("ab \\*cd ef");
        h.caret(at);
        h.key(Qt::Key_Delete, Qt::ControlModifier, QString());
        const QString b = h.body();
        int lone = 0;
        for (int i = 0; i < b.size(); ++i) {
            if (b.at(i) == QLatin1Char('\\')) {
                if (i + 1 < b.size() && ManuscriptEscapes::isEscapable(b.at(i + 1))) {
                    ++i;
                } else {
                    ++lone;
                }
            }
        }
        check(lone == 0, QString("M3: Ctrl+Delete at %1 left an orphaned backslash: %2").arg(at).arg(visible(b)));
        check(!b.contains(QRegularExpression("(^|[^\\\\])\\*")), QString("M3: Ctrl+Delete at %1 left a live *: %2").arg(at).arg(visible(b)));
    }
    // The second scene is never touched by any of the above.
    {
        Harness h("x");
        h.caret(1);
        h.key(Qt::Key_Return, Qt::ShiftModifier, "\r");
        check(h.doc.bodyText(1) == "Tail", "the next scene's body is untouched");
        check(!h.body().contains(QChar(0x2028)), "M1: Shift-Return never stores U+2028");
    }
}

}   // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: escape_smoke <manuscript_format_corpus.json>\n");
        return 2;
    }
    runCorpus(QString::fromLocal8Bit(argv[1]));
    runLinuxOnly();
    if (failures == 0) {
        std::printf("escape-ok\n");
        return 0;
    }
    return 1;
}
