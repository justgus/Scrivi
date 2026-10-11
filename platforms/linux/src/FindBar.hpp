#pragma once

// SP-176 / T-0603 (EP-048 S4, L8's Find half) — the manuscript's FIND BAR and Edit ▸ Find.
//
// ✅ Q4 (ruled 2026-10-11): a bar at the top of the manuscript — where Apple's find bar sits (`NSTextFinder` in the
// manuscript's scroll view, [SP-164]). Qt has none to point at presented text, so this is Scrivi's: the query, the count,
// Previous / Next, Contains · Starts With · Whole Word, Ignore Case (on by default — [SP-164] Q6), Done.
// ⛔ No Replace controls: Replace waits for Linux undo ([EP-026]; SP-176 Q3) and arrives as EP-048 S4b, as a second row.
// Matches are of the PRESENTED text (`ManuscriptFind::PresentedText`), built at the first Find and rebuilt after an edit —
// a writer who never searches never pays for it. Every match in view is highlighted (extra selections: drawing only, the
// document is untouched); the current one is the editor's selection.

#include <QWidget>

#include <functional>
#include <optional>

#include "ManuscriptFind.hpp"

class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QMenu;
class QTimer;
class ManuscriptEditor;
class ManuscriptPresenter;
class SceneDocument;

// The Edit ▸ Find commands (Apple: `ManuscriptFindCommand`).
enum class FindCommand { ShowFind, NextMatch, PreviousMatch, UseSelectionForFind };

// Edit ▸ Find ▸ … — ✅ Q2 (ruled 2026-10-11; Ctrl in place of ⌘): Find… Ctrl+F · Find Next Ctrl+G, F3 · Find Previous
// Ctrl+Shift+G (amended 2026-10-11), Shift+F3 · Use Selection for Find Ctrl+E. One builder, shared with `find_smoke`.
// Returns the actions (the window disables them without a project).
QList<QAction*> addFindMenu(QMenu* edit, QObject* context, std::function<void(FindCommand)> run);

class FindBar : public QWidget
{
    Q_OBJECT

public:
    explicit FindBar(ManuscriptEditor* editor, QWidget* parent = nullptr);

    // Point the bar at the manuscript (after each load). The document's edits make the presented text stale.
    void attach(SceneDocument* doc, ManuscriptPresenter* presenter);

    void run(FindCommand command);
    void closeBar();

    // Tests: the matches of the current query, and the count as shown.
    const QVector<MarkdownRange>& matches();
    QString countText() const;
    QLineEdit* queryField() const { return query_; }
    QComboBox* modeBox() const { return mode_; }
    QCheckBox* ignoreCaseBox() const { return ignoreCase_; }
    int presentedBuilds() const { return builds_; }   // refreshes that rebuilt anything
    int scenesRebuilt() const { return lastRebuilt_; }  // by the last of them

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void showFind();
    void step(bool forward);
    void useSelection();
    void queryChanged();            // incremental: re-search, select the first match from the selection's start
    void onContentsChange(int position, int removed, int added);   // a member slot: Qt::UniqueConnection needs one
    const ManuscriptFind::PresentedText& presented();
    ManuscriptFind::Options options() const;
    void select(int index);
    int currentIndex() const;       // the match the editor's selection IS, or -1
    void updateCount();
    void updateHighlights();

    ManuscriptEditor* editor_;
    SceneDocument* doc_ = nullptr;
    ManuscriptPresenter* presenter_ = nullptr;
    QLineEdit* query_;
    QLabel* count_;
    QComboBox* mode_;
    QCheckBox* ignoreCase_;
    QTimer* refresh_;
    ManuscriptFind::PresentedText presented_;
    bool presentedStale_ = true;
    QVector<MarkdownRange> matches_;
    bool matchesStale_ = true;
    // The match last selected and the selection the editor actually holds for it: the editor SNAPS a selection so it never
    // starts just after hidden markup (L5, Apple's `snapSelection`; design §7) — `bold` in `**bold**` is held as `**bold`.
    int selectedIndex_ = -1;
    MarkdownRange selectedAs_;
    int builds_ = 0;
    int lastRebuilt_ = 0;
};
