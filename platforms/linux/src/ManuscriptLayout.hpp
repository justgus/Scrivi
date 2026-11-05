#pragma once

#include <QPlainTextDocumentLayout>
#include <QTextBlockUserData>

// EP-048 S5b ([SP-183], T-0610) — the manuscript's LAYOUT on Linux: EP-047's fixed line heights (P9) and first-line indent
// (P3, P10), drawn as Apple's paragraph styles draw them, with ZERO characters and no document edit.
//
// ✅ [SP-183] Q1 = (B), from step 0's measurement: `QPlainTextDocumentLayout` IGNORES a block format's `textIndent` and
// `lineHeight` (Qt 6.4.2 and 6.10.2 alike), and setting one is a document edit (`contentsChange`, the modified flag), which
// would dirty and save the scene. So the layout itself places each line: it re-lays a block Qt has just laid out, with the
// first line indented and every line on a fixed pitch.
// What each line gets is decided by the PRESENTER while it highlights (it knows headings, lists and scene edges) and left
// on the block as `ManuscriptLineStyle` — user data, which Qt never reports as an edit. A block with no style yet (before
// its first highlight) keeps Qt's own layout.
struct ManuscriptLineStyle : QTextBlockUserData
{
    double indent = 0;   // the first line's indent, px
    double pitch = 0;    // every line's height, px (0 = Qt's own layout)
    // Where our layout left the first line, AS QT REPORTS IT BACK: positions are stored in 1/64 px, so an exact compare with
    // these (never with values computed here) is what says "still ours". ⛔ [SP-183] live pass: a computed compare failed on
    // every call, and each re-lay shrank the paragraph by the rounding.
    double laidX = -1;
    double laidY = -1;
};

class ManuscriptLayout : public QPlainTextDocumentLayout
{
public:
    using QPlainTextDocumentLayout::QPlainTextDocumentLayout;

    QRectF blockBoundingRect(const QTextBlock& block) const override;

    // For the smokes ([I-0351]): how many blocks were re-laid, and how often a block's rectangle was asked for.
    [[nodiscard]] int relayouts() const { return relayouts_; }
    [[nodiscard]] int boundingRectCalls() const { return calls_; }

private:
    mutable double availableWidth_ = 0;   // Qt's line width (its `layoutBlock`: the text width less a margin each side)
    mutable int relayouts_ = 0;
    mutable int calls_ = 0;
};
