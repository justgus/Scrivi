#pragma once

// EP-049 (SP-160, T-0586) — the manuscript escape rules on Linux.
//
// ✅ A PORT of Apple's `Scrivi/Views/ManuscriptEscapes.swift` (EP-045 AC3/AC4, Q1 = (a)), rule for rule:
// "do what Apple does, the way Apple does it" (user, 2026-10-04). ⛔ Not a new design — a change to the
// rules is made on Apple first and mirrored here in the same work.
// ⚠️ The shared corpus (`ScriviCore/tests/fixtures/manuscript_format_corpus.json`, EP-049 AC8) runs on
// BOTH platforms; it is what catches the two copies drifting.
//
// ✅ SOURCE    = every stored character, escape backslashes included (what the .md holds).
// ✅ PRESENTED = the source with each ESCAPING backslash removed (`\*` presents as `*`).
// ⚠️ Offsets are UTF-16 code units (QString indices), as Apple's are NSRange units.

#include <QString>
#include <QVector>

namespace ManuscriptEscapes {

// ✅ The ruled set — all 32 ASCII punctuation marks (study §4B.4). ⚠️ ONE list serves both directions.
bool isEscapable(QChar c);

// The WRITE half: every escapable mark gets a backslash. `map(escape(t)).presented == t` for every `t`.
QString escape(const QString& typed);

// Boundary maps for one fragment (caret BOUNDARIES 0...length, so both carry a final entry).
struct Map {
    QVector<int> presentedToSource;   // ⚠️ an escaped mark maps to BEFORE its backslash
    QVector<int> sourceToPresented;   // the boundary between `\` and its mark → before the mark
    QString presented;
};

// `continues`: whether a non-blank line follows the fragment — only matters when it ends in `\` + `\n`
// (Q1 = (a): a hard break cannot END a paragraph, so `\` before a blank line or the end stays literal).
Map map(const QString& source, bool continues = false);

// Offsets of every backslash HIDDEN when presented (an escape, or a hard line break).
QVector<int> hiddenBackslashes(const QString& source, bool continues = false);

// A CommonMark blank line: only spaces and tabs (and newlines).
bool isBlankLine(const QString& line);

}   // namespace ManuscriptEscapes
