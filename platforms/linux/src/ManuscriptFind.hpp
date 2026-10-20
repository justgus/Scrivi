#pragma once

// SP-176 / T-0603 (EP-048 S4, L8's Find half) — FIND OVER WHAT THE WRITER SEES.
//
// ✅ The Linux shape of Apple's `PresentedText` (`Scrivi/Views/ManuscriptPresentedMap.swift`, [SP-164] / [SP-171]): the
// manuscript AS THE PAGE SHOWS IT AT REST — escape backslashes, emphasis markers and heading prefixes are not in it (Q-E2-5);
// list prefixes are visible, so they are. ✅ Q1 (ruled 2026-10-11): built from the presenter's per-block analysis, the rule
// Linux already draws with (`ManuscriptPresenter::hiddenAtRest`).
// ✅ One chunk per SCENE body: chapter titles and dividers are not in it ([SP-164] Q4), and a match never crosses a scene.
// Every presented unit maps back to its DOCUMENT position, so a match is a document range to select.
// ✅ Kept PER SCENE and refreshed per scene: an edit rebuilds only the scene it touched (measured 2026-10-11: a whole build of
// ~1.86 MB with unique paragraphs is ~290 ms — fine once, not after every pause in typing with the bar open). Apple's map is
// patched per edit for the same reason ([SP-171] Plan 1(c)).
// ⛔ No widgets here — `find_smoke` tests it directly; `FindBar` drives it.

#include <QString>
#include <QVector>

#include "MarkdownAnalysis.hpp"

class QTextDocument;

class ManuscriptPresenter;
class SceneDocument;

namespace ManuscriptFind {

// [SP-164] Q6 (Apple's find bar): Contains · Starts With · Whole Word; Ignore Case on by default.
enum class Mode { Contains, StartsWith, WholeWord };
struct Options {
    Mode mode = Mode::Contains;
    bool ignoreCase = true;
};

class PresentedText
{
public:
    // The whole manuscript. `presenter` nullptr = drawn as stored (nothing hidden).
    static PresentedText build(const SceneDocument& doc, const ManuscriptPresenter* presenter);

    // Bring it up to date with `doc`: a scene is rebuilt when it was marked edited, or its id or body length no longer
    // match (a structural change — insert, delete, move — shows up as one of those). Returns how many scenes were rebuilt.
    int refresh(const SceneDocument& doc, const ManuscriptPresenter* presenter);
    // ✅ [T-0613]: the same, a SLICE at a time — from scene `from`, until `budgetMs` has passed (at least one scene).
    // Returns the scene to resume from; at `doc.segments().size()` call `refresh()` once more, which then only checks.
    int refreshSlice(const SceneDocument& doc, const ManuscriptPresenter* presenter, int from, int budgetMs);
    // An edit at document position `pos` (after the scene map was reconciled): its scene is rebuilt at the next refresh.
    void markEdited(const SceneDocument& doc, int pos);

    // Every match, in document order, as DOCUMENT ranges. A match that starts at an escaped mark takes its hidden `\`,
    // so selecting it never splits an escape pair; its end is just after its last presented character, so a closing
    // marker that FOLLOWS it is not in it (Apple's `storageRange`).
    QVector<MarkdownRange> find(const QString& query, const Options& options) const;

    // The presented text of document range [start, end) — Use Selection for Find. Scenes joined by '\n'.
    QString presentedOf(int start, int end) const;

    // The presented units (tests and the measurement).
    int length() const;

private:
    struct Scene {
        QString sceneID;
        int bodyStart = 0;     // refreshed every time — an edit in an earlier scene moves it
        int bodyLength = -1;
        bool edited = true;
        QString text;          // as presented
        QVector<int> rel;      // position of each presented unit, relative to bodyStart
        QVector<bool> escaped; // the unit's hidden escape backslash sits just before it
    };
    static void buildScene(Scene& scene, QTextDocument* doc, int bodyStart, int bodyLength,
                           const ManuscriptPresenter* presenter);
    // Brings scene `i` up to date with segment `i`; true when it was rebuilt.
    bool refreshScene(int i, const SceneDocument& doc, QTextDocument* text, const ManuscriptPresenter* presenter);
    QVector<Scene> scenes_;
};

}   // namespace ManuscriptFind
