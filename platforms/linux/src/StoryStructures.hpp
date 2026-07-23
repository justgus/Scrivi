#pragma once

#include <QList>
#include <QString>

// StoryStructures — the built-in story-structure presets (EP-025 / SP-081, T-0329).
//
// Ported verbatim from Apple's BuiltInStructure.bandTable (TimelineStripView.swift):
// each structure is an ordered list of proportional colored bands. The app OWNS these
// presets; ScriviCore just stores whatever bandLayoutJSON it's handed
// (scrivi_set_story_structure), so a "custom" layout is just an edited preset — no new
// schema. Border-drag re-proportion (T-0331) edits the proportions and re-persists via
// scrivi_update_band_layout.
namespace scrivi::linux_app::story {

struct Band {
    QString bandID;
    QString label;
    QString color;        // hex "#RRGGBB"
    double  proportion;   // 0 < p < 1; the bands of a structure sum to 1.0
};

struct Structure {
    QString      structureID;   // e.g. "three-act"
    QString      name;          // display name, e.g. "Three Act Structure"
    QList<Band>  bands;
};

// All built-in structures, in menu order.
const QList<Structure>& builtInStructures();

// The structure with `structureID`, or nullptr if unknown.
const Structure* structureForID(const QString& structureID);

// Serialise a band list to the bandLayoutJSON the C ABI expects: a JSON array of
// {bandID, label, color, proportion}. Used for set_story_structure + update_band_layout.
QString bandLayoutJSON(const QList<Band>& bands);

// Parse a bandLayoutJSON string (from get_story_structure) back into a band list.
// Returns an empty list on malformed input.
QList<Band> parseBandLayout(const QString& json);

} // namespace scrivi::linux_app::story
