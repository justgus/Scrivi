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
// ⚠️ P9's fixed 1.45 line height is a BLOCK format on Qt → [SP-183] (Q3); Qt's default line spacing until then.
struct ManuscriptTypography {
    static constexpr double kDefaultSize = 16.0;
    static constexpr double kMinSize = 10.0;
    static constexpr double kMaxSize = 32.0;

    // The face's NAME as stored in `project-settings.json` — kept even when this build does not bundle it.
    QString faceName;
    double size = kDefaultSize;

    ManuscriptTypography();   // the defaults (P6, P9)
    ManuscriptTypography(QString faceName, double size);

    // The face drawn — the stored one, or the default if this build does not bundle it.
    const BundledFonts::Face* face() const { return BundledFonts::face(faceName); }
    // The body font: the face drawn, at `size`.
    QFont bodyFont() const;
};
