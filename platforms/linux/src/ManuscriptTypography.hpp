#pragma once

#include <QFont>
#include <QString>
#include <QVector>

// EP-048 S5a (SP-182, T-0609) — the manuscript's TYPE on Linux: the bundled faces, and the one source every type decision
// reads. ✅ Apple's shape (`Scrivi/Views/ManuscriptTypography.swift`): `BundledFonts` + `ManuscriptTypography`.

// The typefaces Scrivi ships (EP-047 P5a). ✅ The list is `Resources/Fonts/fonts.json`, compiled into the app as a Qt
// resource with every file it names ([SP-182] Q1) — the SAME file the Apple app reads. ⛔ Never restate the faces in code.
namespace BundledFonts {

struct FontFile {
    QString file;
    bool italic = false;
    int weight = 400;
};

struct Face {
    QString name;
    QString role;
    QString folder;
    bool variable = false;
    QVector<FontFile> files;
    QString family;   // the family Qt registered the files under (empty until `registerAll`, or if it failed)
};

struct Manifest {
    QString defaultFace;
    QVector<Face> faces;
};

// `:/fonts/fonts.json`, read once. Empty (no faces) when the resource is missing — the manuscript then draws in the
// system font.
const Manifest& manifest();

// Registers every bundled font for THIS process (`QFontDatabase::addApplicationFont`). Call once, after the
// QGuiApplication exists. Safe to call again (a no-op).
void registerAll();

// The face called `name`, or the default face (P6) when no bundled face has that name — a project set in a face from a
// LATER Scrivi still opens. ⚠️ The caller keeps the stored name; only the drawing falls back. nullptr when nothing is bundled.
const Face* face(const QString& name);

}  // namespace BundledFonts

// THE one source of the manuscript's type (EP-047 AC4): the editor's font and the document's default font come from here,
// and the presenter sizes headings, emphasis and chapter titles from that default. ⛔ A second source is the trap EP-047
// recorded: a face that vanishes on undo, on reload or on Replace All.
// ✅ Rulings: P2/P8 per project, P5a bundled faces, P6 default Literata, P7 headings and chapter titles in the face, P9 16 pt.
// ✅ P3, P9, P10 ([SP-183]): the fixed line heights and the first-line indent are drawn by `ManuscriptLayout` (Q1: a layout,
// never a block format, which Qt's plain layout ignores and which is a document edit).
struct ManuscriptTypography {
    static constexpr double kDefaultSize = 16.0;
    static constexpr double kMinSize = 10.0;
    static constexpr double kMaxSize = 32.0;

    // ✅ EP-047 S3 (P3, P10): which body paragraphs get a first-line indent — Apple's `ParagraphIndent`, stored by its raw
    // value (`book` · `every` · `none`; absent or unknown = `book`).
    enum class ParagraphIndent { Book, Every, None };
    static ParagraphIndent indentFromName(const QString& name);
    static constexpr double kDefaultIndentEm = 1.5;
    static constexpr double kMinIndentEm = 0.5;
    static constexpr double kMaxIndentEm = 4.0;
    // P9: body lines sit on a FIXED 1.45 × size line, the same for every face; headings and chapter titles on 1.25 × their
    // own size. P10: with an indent on, a stored blank line between paragraphs draws at ⅓ of a body line.
    static constexpr double kLineSpacing = 1.45;
    static constexpr double kHeadingLineSpacing = 1.25;
    static constexpr double kGapFraction = 1.0 / 3.0;

    // The face's NAME as stored in `project-settings.json` — kept even when this build does not bundle it.
    QString faceName;
    double size = kDefaultSize;
    ParagraphIndent indent = ParagraphIndent::Book;
    double indentEm = kDefaultIndentEm;   // in ems: it follows the size and the face

    ManuscriptTypography();   // the defaults (P6, P9)
    ManuscriptTypography(QString faceName, double size, ParagraphIndent indent = ParagraphIndent::Book,
                         double indentEm = kDefaultIndentEm);

    // The face drawn — the stored one, or the default if this build does not bundle it.
    const BundledFonts::Face* face() const { return BundledFonts::face(faceName); }
    // The body font: the face drawn, at `size`.
    QFont bodyFont() const;
};
