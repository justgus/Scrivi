#pragma once

#include <QHash>
#include <QSet>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

#include <functional>
#include <optional>

#include "MarkdownAnalysis.hpp"

class SceneDocument;

// EP-048 L3–L6, L9 part (SP-166, T-0595) — the Linux manuscript PRESENTER.
//
// ✅ The Qt shape of Apple's `ManuscriptPresenter` (`Scrivi/Views/ManuscriptPresenter.swift`, route (a′)): the
// document holds the STORED text and nothing about it changes — a `QSyntaxHighlighter`'s formats are
// presentation-only (W3, design §2.4), so the save path never sees them. ⛔ Not a new design: a rule changed
// on Apple is changed here in the same work (`feedback_linux_adopts_apple_shape`).
//
// What it draws, per Markdown block (a maximal run of non-blank lines of ONE scene):
//   • escape and hard-break backslashes — always hidden (E1, `ManuscriptEscapes::hiddenBackslashes`);
//   • heading prefixes — hidden, or shown dimmed on a line holding a selection end (Q-E2-1 line half);
//   • emphasis markers — hidden, or shown dimmed while a selection end is at the span's first or last
//     character (Q-E2-1 span half, as amended by the SP-162 live pass);
//   • heading size/weight, bold, italic; list prefixes visible and dimmed.
// What the analyzer demotes (code blocks, fences, tables, nested headings — EP-045 AC7, [I-0280]) is drawn
// as stored.
//
// ⚠️ A Qt block is a LINE; a Markdown block is a RUN of lines. An edit on one line can change how its
// siblings render (`*a⏎b*`), so every line of an edited Markdown block is re-highlighted — Apple's
// "widen an edit to its block" (design §3.5).
class ManuscriptPresenter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    using Analyzer = std::function<MarkdownAnalysis(const QString&)>;

    explicit ManuscriptPresenter(Analyzer analyzer, QObject* parent = nullptr);
    // ⚠️ Detaches HERE: `~QSyntaxHighlighter` clears the document's formats in an edit block, which emits
    // `contentsChange` into our slot after this object is gone — "pure virtual method called" (measured).
    ~ManuscriptPresenter() override;

    // Draw `document` (nullptr = detach). ⚠️ Use this, not `setDocument`: it also moves the edit hook.
    // Qt highlights the whole document on the NEXT event-loop turn after attaching.
    void attach(QTextDocument* document);

    // Blocks never cross a scene edge (chapter headings and separators are not scene text).
    // nullptr = the whole document is one scene (tests).
    void setSceneDocument(const SceneDocument* doc) { sceneDoc_ = doc; }

    // A Markdown block: document positions [start, end) of its text (lines joined by '\n'; the
    // last line's own newline included when one follows, as Apple's paragraph ranges include it).
    struct Block { int start = 0; int end = 0; QString text; };
    std::optional<Block> blockAt(int pos) const;

    // ── Where the caret may rest (Apple: `stopTest`) ──
    // A STOP RUN is markup the caret never rests inside; its HOME is the side the caret goes to:
    // escape → before · opener, heading prefix, list prefix → AFTER · closer → before (design §3.6).
    enum class StopKind { Escape, Opener, Closer, Prefix, ListPrefix };
    struct Stop {
        int start = 0;
        int end = 0;
        StopKind kind = StopKind::Escape;
        bool homeAfter() const { return kind == StopKind::Opener || kind == StopKind::Prefix || kind == StopKind::ListPrefix; }
    };
    // The stop run holding document position `i` (the character AT i), if any.
    std::optional<Stop> stopAt(int i) const;

    // ── The reveal (Q-E2-1) ──
    // Move the reveal to a selection [selStart, selEnd]; re-highlights only the lines whose drawing changes.
    void reveal(int selStart, int selEnd);
    // True when the character at `i` is drawn hidden under the CURRENT reveal (tests, and Apple's `isHidden`).
    bool isHidden(int i) const;

    // Body text height multipliers, Apple's SP-161 Q1 sizes over its 13 pt body: H1 22 · H2 18 · H3 16.
    static double headingScale(int level);

    int analyzerCalls() const { return analyzerCalls_; }

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Info { MarkdownAnalysis a; QSet<int> escapes; };   // block-relative
    const Info& info(const QString& blockText) const;
    int sceneOf(const QTextBlock& b) const;
    QVector<MarkdownRange> revealSpans(int selStart, int selEnd) const;
    bool lineRevealed(int blockNumber) const { return revealedLines_.contains(blockNumber); }
    void rehighlightRange(int from, int to);   // every line of every Markdown block touching [from, to]
    void onContentsChange(int from, int removed, int added);

    QTextCharFormat hiddenFormat() const;
    QTextCharFormat dimmedFormat() const;

    Analyzer analyzer_;
    const SceneDocument* sceneDoc_ = nullptr;
    mutable QHash<QString, Info> cache_;      // keyed by block TEXT — no invalidation needed (§3.5)
    mutable int analyzerCalls_ = 0;
    QSet<int> revealedLines_;                 // Qt block numbers holding a selection end
    QVector<MarkdownRange> revealedSpans_;    // document positions
    bool rehighlighting_ = false;
    QMetaObject::Connection edits_;
};
