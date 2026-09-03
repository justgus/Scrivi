#include "SceneInspector.hpp"

#include "ThemeColours.hpp"
#include "WorldStatusText.hpp"

#include "ObjectKindScope.hpp"
#include "ScriviBridge.hpp"

#include <QAction>
#include <QFont>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSet>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QVariantList>
#include <QVariantMap>
#include <QEvent>
#include <QSignalBlocker>
#include <QStringList>

#include <algorithm>

// The panel lives in EditorShell's QSplitter, so the minimum is a hard floor and
// the default is applied by the splitter's initial sizes.
//
// ⚠️ WIDENED 2026-08-30 (user ruling) from 120/200 to 240/400. The earlier
// 120/200 came from a 2026-07-22 preference for a wider writing surface — ⚠️ but
// that predates the THREE-TAB shell, and at 200px the three tab labels truncate
// to "W… / Worldb… / Prop…". ✅ **"You need at least that much to view all three
// tabs without truncating."**
//
// ⚠️ Label elision (setElideMode, below) is still kept as the SAFETY NET, not as
// the answer: a writer may drag the splitter narrower, and the tabs must stay
// clickable rather than collapsing into scroll arrows when she does.
namespace {
constexpr int kMinWidth     = 240;
constexpr int kDefaultWidth = 400;

// Roles carrying an entry's identity on its tree item. A kind GROUP row carries
// none of these, which is what distinguishes it from an object row.
constexpr int kRoleObjectID = Qt::UserRole + 1;
constexpr int kRoleKind     = Qt::UserRole + 2;
constexpr int kRoleWorldID  = Qt::UserRole + 3;
constexpr int kRolePending  = Qt::UserRole + 4;
// ⚠️ The object's display name ALONE. The row's visible text is
// "<name> — <label>" (plus a ⚠ badge when pending), so recovering the name by
// string-surgery on the row text is wrong — and it shipped wrong: a pending
// message read «"Myton at 23 — features" can't be opened», with the EDGE LABEL
// glued to the character's name. ✅ Carry the name as data instead of parsing
// the presentation back apart.
constexpr int kRoleDisplayName = Qt::UserRole + 5;
} // namespace

SceneInspector::SceneInspector(QWidget* parent) : QWidget(parent)
{
    setMinimumWidth(kMinWidth);
    // A preferred width so the enclosing splitter opens the panel at Apple's
    // default; the splitter's setSizes() in EditorShell is the authority, but this
    // gives the widget a sensible sizeHint if that ever changes.
    resize(kDefaultWidth, height());

    tabs_ = new QTabWidget(this);
    // North tab bar mirrors Apple's segmented Picker sitting above the tab body.
    tabs_->setTabPosition(QTabWidget::North);
    tabs_->setDocumentMode(true);

    // ⚠️ ADDED IN DISPLAY ORDER, and the order is Apple's, not a preference:
    // the tab bar iterates `InspectorTab.allCases`, which follows the enum's
    // declaration order `writing, worldbuilding, properties`
    // (InspectorCard.swift:19-22). The indices must match the Tab enum.
    tabs_->addTab(buildWritingTab(),       tr("Writing"));
    tabs_->addTab(buildWorldbuildingTab(), tr("Worldbuilding"));
    tabs_->addTab(buildPropertiesTab(),    tr("Properties"));
    tabs_->setCurrentIndex(TabWriting);   // Apple's default until a layout loads

    // ⚠️ At the panel's 200px default width, three full tab labels do NOT fit,
    // and QTabWidget's answer is a pair of ~8px scroll arrows. ⚠️ That makes
    // Properties reachable ONLY by a tiny arrow target — which is a gesture-only
    // affordance in all but name, and fails the same rule as T-0482
    // (`project_linux_vnc_input_constraints`: every action needs a real target,
    // and a remote pass may drop precision as well as modifiers).
    //
    // ✅ Elide the labels instead: all three tabs stay visible and clickable at
    // any width, shortening rather than disappearing. ⚠️ Found by LOOKING at the
    // panel — the build was green and every smoke passed with the tab
    // unreachable, because no test renders a tab bar.
    tabs_->setUsesScrollButtons(false);
    tabs_->setElideMode(Qt::ElideRight);

    connect(tabs_, &QTabWidget::currentChanged, this, &SceneInspector::onTabChanged);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(tabs_);
}

QString SceneInspector::tabID(int index)
{
    switch (index) {
        case TabWriting:       return QStringLiteral("writing");
        case TabWorldbuilding: return QStringLiteral("worldbuilding");
        case TabProperties:    return QStringLiteral("properties");
    }
    return QStringLiteral("writing");
}

int SceneInspector::tabIndex(const QString& id)
{
    if (id == QLatin1String("worldbuilding")) { return TabWorldbuilding; }
    if (id == QLatin1String("properties"))    { return TabProperties; }
    return TabWriting;   // unknown degrades to the ruled default
}

void SceneInspector::onTabChanged(int index)
{
    // ⚠️ PROJECT-level, and deliberately not per-scene: switching scenes reloads
    // the current tab's content, it never changes which tab is showing.
    layout_.setSelectedTab(tabID(index));
}

QWidget* SceneInspector::buildWritingTab()
{
    // T-0487 — the Writing stack: tags, outline, todo. Apple ships these three
    // as the DEFAULT writing cards for a fresh project
    // (InspectorLayoutStore.swift:81-85), so they are the right three here.
    //
    // ⚠️ Rendered as a plain form, NOT as Apple's collapsible card stack. Card
    // collapse/reorder/sort is T-0491 and is user-ruled OUT of this sprint; the
    // layout file's `stackSort` and `defaultStacks` are round-tripped untouched
    // meanwhile, so nothing is lost by the simpler presentation.
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(6);

    auto boldLabel = [tab](const QString& text) {
        auto* l = new QLabel(text, tab);
        QFont f = l->font();
        f.setBold(true);
        l->setFont(f);
        return l;
    };

    layout->addWidget(boldLabel(tr("Tags")));
    tagsEdit_ = new QLineEdit(tab);
    tagsEdit_->setPlaceholderText(tr("comma, separated, tags"));
    // Commit on editing-finished (Return or focus-out), not per keystroke — a
    // scene note is not worth a disk write per character.
    connect(tagsEdit_, &QLineEdit::editingFinished, this, &SceneInspector::onTagsEdited);
    layout->addWidget(tagsEdit_);

    layout->addWidget(boldLabel(tr("Outline")));
    outlineEdit_ = new QTextEdit(tab);
    outlineEdit_->setAcceptRichText(false);   // the core stores plain text
    outlineEdit_->setPlaceholderText(tr("What happens in this scene?"));
    // ⚠️ Commit on focus-out — see eventFilter(). QTextEdit has no
    // editingFinished, so without this the note is only saved when the scene
    // changes, and typing then clicking into the manuscript LOSES it.
    outlineEdit_->installEventFilter(this);
    layout->addWidget(outlineEdit_, 2);

    layout->addWidget(boldLabel(tr("To-do")));
    todoEdit_ = new QTextEdit(tab);
    todoEdit_->setAcceptRichText(false);
    // ⚠️ One item per line; a leading "[x] " marks it done. The core models todo
    // as {text, done} pairs, so this is a TEXT PRESENTATION of that structure —
    // the parse is explicit in onTodoEdited() rather than implied here.
    todoEdit_->setPlaceholderText(tr("One item per line.\n[x] done item"));
    todoEdit_->installEventFilter(this);   // commit on focus-out, as above
    layout->addWidget(todoEdit_, 2);

    writingStatus_ = new QLabel(tab);
    writingStatus_->setWordWrap(true);
    writingStatus_->setEnabled(false);
    layout->addWidget(writingStatus_);

    return tab;
}

QWidget* SceneInspector::buildWorldbuildingTab()
{
    // T-0489 — SP-125's object list, MOVED here unchanged. ⚠️ This is the tab it
    // always belonged in; "Scene Entities" was never a real tab (see the header).
    // ⚠️ NO behaviour change: same read path, same two open affordances, same
    // four explained states.
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    title_ = new QLabel(tr("Objects in this scene"), tab);
    QFont titleFont = title_->font();
    titleFont.setBold(true);
    title_->setFont(titleFont);
    title_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(title_);

    tree_ = new QTreeWidget(tab);
    tree_->setHeaderHidden(true);
    tree_->setColumnCount(1);
    tree_->setRootIsDecorated(true);
    // Rows are single-line again (the label moved to the header), so uniform
    // heights are both correct and faster.
    tree_->setUniformRowHeights(true);
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    // T-0482: the context menu is a FIRST-CLASS path to open, not a convenience
    // duplicate of the double-click. The VNC/RDP input path carries no
    // Shift-combos and no trackpad gestures, so every action needs a button or
    // menu route. ⚠️ The rig does NOT retire this rule — remote passes may still
    // drop modifiers.
    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree_, &QTreeWidget::itemDoubleClicked,
            this, &SceneInspector::onItemActivated);
    // itemActivated covers Return/Enter — the same open, reachable with no
    // pointer at all.
    connect(tree_, &QTreeWidget::itemActivated,
            this, &SceneInspector::onItemActivated);
    connect(tree_, &QTreeWidget::customContextMenuRequested,
            this, &SceneInspector::onContextMenuRequested);
    layout->addWidget(tree_, 1);

    // ⚠️ T-0483: the panel's one explanatory line. Empty, unavailable, pending
    // and unreadable-index are DIFFERENT states and each gets its own sentence.
    // It is never left blank while the tree is empty.
    status_ = new QLabel(tab);
    status_->setWordWrap(true);
    status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(status_);

    setStatusLine(tr("No project open."), false);
    return tab;
}

QWidget* SceneInspector::buildPropertiesTab()
{
    // T-0488 — Apple's Properties tab is a FIXED view, not a card stack
    // (`InspectorTab.stack` returns nil for it), showing derived scene facts.
    //
    // ⚠️ Everything here is READ-ONLY IN FACT, not merely styled. QLabel cannot
    // be edited at all, which is the point: I-0148 recorded a disabled SwiftUI
    // `TextEditor` on Apple that was STILL EDITABLE. A read-only state must be
    // ENFORCED by the widget choice, never by an `setEnabled(false)` that a
    // future refactor can quietly flip.
    //
    // ⚠️ Costs NO extra backend call — every value comes from the same
    // `getSceneNotes` result the Writing tab already reads.
    auto* tab = new QWidget;
    auto* outer = new QVBoxLayout(tab);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(8);

    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    auto makeValue = [tab]() {
        auto* l = new QLabel(tab);
        l->setWordWrap(true);
        // Selectable so a writer can copy a timestamp; still not editable.
        l->setTextInteractionFlags(Qt::TextSelectableByMouse);
        return l;
    };

    propTitle_    = makeValue();
    propWords_    = makeValue();
    propChars_    = makeValue();
    propCreated_  = makeValue();
    propModified_ = makeValue();

    form->addRow(tr("Title"),         propTitle_);
    form->addRow(tr("Words"),         propWords_);
    form->addRow(tr("Characters"),    propChars_);
    form->addRow(tr("Created"),       propCreated_);
    form->addRow(tr("Last modified"), propModified_);
    outer->addLayout(form);
    outer->addStretch(1);

    propStatus_ = new QLabel(tab);
    propStatus_->setWordWrap(true);
    propStatus_->setEnabled(false);
    outer->addWidget(propStatus_);

    return tab;
}

void SceneInspector::setContext(ScriviBridge* bridge, const QString& projectRootPath)
{
    bridge_          = bridge;
    projectRootPath_ = projectRootPath;
    // The scene is set separately, by whichever hook reports the active scene.
    sceneID_.clear();
    entries_.clear();
    worldNames_.clear();
    loadError_.clear();

    // T-0486: restore the PROJECT's tab selection. ⚠️ Loading the layout must not
    // itself look like a writer choosing a tab, or opening a project would
    // rewrite the file with the value it just read — so the signal is blocked.
    layout_.load(projectRootPath);
    if (tabs_ != nullptr) {
        const QSignalBlocker block(tabs_);
        tabs_->setCurrentIndex(tabIndex(layout_.selectedTab()));
    }

    rebuildTree();
    loadSceneNotes();
}

void SceneInspector::setScene(const QString& sceneID)
{
    if (sceneID == sceneID_) {
        return;   // scroll traffic re-reports the same scene constantly
    }
    // ⚠️ Flush BEFORE the scene changes. The Writing tab commits on focus-out,
    // and clicking from a note straight into another scene never fires that —
    // so without this the edit is lost. Same shape as I-0119 on Apple, where a
    // draft completed at a scene boundary attached to the wrong scene.
    flushPendingEdits();

    sceneID_ = sceneID;
    reload();
    loadSceneNotes();
}

void SceneInspector::reload()
{
    entries_.clear();
    loadError_.clear();
    indexUnreadable_ = false;

    if (bridge_ == nullptr || projectRootPath_.isEmpty()) {
        rebuildTree();
        return;
    }
    if (sceneID_.isEmpty()) {
        rebuildTree();
        return;
    }

    // 1 — the SCENE's edges. Each carries its far endpoint already resolved,
    //     including the label projected for THIS endpoint.
    const QVariantMap edgesResult = bridge_->listEdgesFor(projectRootPath_, sceneID_);
    // ⚠️ An empty map does NOT mean "no edges". JsonDoc::appendToArray creates
    // the "edges" key only when there is a first element to push, so a scene with
    // no edges produces {"ok":true,"result":{}} — byte-identical to what
    // parseEnvelope returns for a FAILED call. Testing isEmpty() here would
    // report every objectless scene as unreadable, which is the same
    // silent-conflation of states, just inverted. Ask the bridge which it was.
    if (bridge_->lastCallFailed()) {
        loadError_ = tr("This scene's objects could not be read.");
        rebuildTree();
        return;
    }

    // 2 — one object listing per load (not a lookup per edge), which also carries
    //     each object's world. Kind filter empty = every kind.
    QSet<QString> knownIDs;
    QHash<QString, QString> worldByObjectID;
    indexUnreadable_ = false;
    const QVariantMap objectsResult = bridge_->listObjects(projectRootPath_, QString());
    const bool objectsReadable = !bridge_->lastCallFailed();
    indexUnreadable_ = !objectsReadable;
    if (objectsReadable) {
        const QVariantList objects = objectsResult.value(QStringLiteral("objects")).toList();
        for (const QVariant& o : objects) {
            const QVariantMap m = o.toMap();
            const QString objectID = m.value(QStringLiteral("objectID")).toString();
            if (objectID.isEmpty()) {
                continue;
            }
            knownIDs.insert(objectID);
            worldByObjectID.insert(objectID, m.value(QStringLiteral("worldID")).toString());
        }
    }
    // ⚠️ A failed listing is NOT fatal — the edges already carry enough to draw
    // every row — but it MUST NOT be allowed to look like an empty scene. With
    // knownIDs empty, the confirmation filter below would drop every non-pending
    // edge and the panel would report "no objects are linked to this scene",
    // which is a false claim about the writer's data, not a degraded one. When
    // the index is unreadable the filter is skipped and every edge is drawn from
    // what the graph says.

    // 3 — bound worlds, so a pending world can be NAMED rather than warned about
    //     anonymously.
    worldNames_.clear();
    const QVariantMap worldsResult = bridge_->listWorlds(projectRootPath_);
    if (!bridge_->lastCallFailed()) {
        const QVariantList worlds = worldsResult.value(QStringLiteral("worlds")).toList();
        for (const QVariant& w : worlds) {
            const QVariantMap m = w.toMap();
            worldNames_.insert(m.value(QStringLiteral("worldID")).toString(),
                               m.value(QStringLiteral("displayName")).toString());
        }
    }

    const QVariantList edges = edgesResult.value(QStringLiteral("edges")).toList();
    for (const QVariant& e : edges) {
        const QVariantMap m = e.toMap();

        Entry entry;
        entry.edgeID        = m.value(QStringLiteral("edgeID")).toString();
        entry.objectID      = m.value(QStringLiteral("otherID")).toString();
        entry.kind          = m.value(QStringLiteral("otherKind")).toString();
        entry.displayName   = m.value(QStringLiteral("otherDisplayName")).toString();
        // ⚠️ READ, never recompute. The core resolved this label for the scene we
        // asked about; deriving direction in Qt is a known trap for this Epic.
        entry.label         = m.value(QStringLiteral("label")).toString();
        entry.sortIndex     = m.value(QStringLiteral("sortIndex")).toDouble();
        entry.worldID       = m.value(QStringLiteral("otherWorldID")).toString();
        entry.pending       = m.value(QStringLiteral("otherPending")).toBool();
        entry.pendingStatus = m.value(QStringLiteral("otherWorldStatus")).toString();

        if (entry.objectID.isEmpty()) {
            continue;
        }
        // ⚠️ A pending endpoint is ABSENT from the object listing (its world is
        // away), so it cannot be confirmed against the index — and dropping it
        // would silently hide it, which §7.2 forbids. It is admitted on the edge's
        // own cached `otherKind` instead.
        const bool known = knownIDs.contains(entry.objectID);
        if (objectsReadable && !known && !entry.pending) {
            // Neither in the index nor claimed pending: the core does not vouch
            // for this endpoint as an object. A scene-to-scene edge lands here
            // legitimately — this tab lists OBJECTS. Only applied when the index
            // was actually READ; see above.
            continue;
        }
        if (entry.kind.isEmpty()) {
            continue;   // nothing to group it under
        }
        // A confirmed object's world comes from the index, which is authoritative;
        // the edge's copy is the binding's cache and is only what we have when the
        // world is away.
        if (known) {
            entry.worldID = worldByObjectID.value(entry.objectID, entry.worldID);
        }
        entries_.append(entry);
    }

    rebuildTree();
}

void SceneInspector::rebuildTree()
{
    if (tree_ == nullptr) {
        return;
    }
    tree_->clear();

    // ⚠️ T-0480: kind order and the world-scope partition come from
    // ObjectKindScope, which asks scrivi_list_object_kinds. NO kind name is
    // written down in this file — a hardcoded list here would be occurrence NINE
    // of the restated-kind-list defect, and occurrence five was in Swift, so a
    // new platform layer is exactly where it recurs.
    const QList<ObjectKindScope::KindInfo> kinds = ObjectKindScope::kinds(bridge_);

    // Group entries by kind, then emit groups in the CORE's kind order so the
    // panel's ordering is the model's, not this file's.
    QHash<QString, QList<Entry>> byKind;
    for (const Entry& e : entries_) {
        byKind[e.kind].append(e);
    }

    QStringList orderedKinds;
    for (const ObjectKindScope::KindInfo& info : kinds) {
        orderedKinds.append(info.kind);
    }
    // A kind the core did not list still gets shown rather than dropped — the
    // panel reports what the project holds even if the two ever disagree.
    QStringList extraKinds = byKind.keys();
    extraKinds.sort();
    for (const QString& k : extraKinds) {
        if (!orderedKinds.contains(k)) {
            orderedKinds.append(k);
        }
    }

    int shown = 0;
    for (const QString& kind : orderedKinds) {
        auto it = byKind.constFind(kind);
        if (it == byKind.constEnd() || it->isEmpty()) {
            continue;   // a kind with nothing in this scene is simply not a row
        }
        QList<Entry> rows = *it;
        // Manual order is the graph's sortIndex (Object Model §5.2) — that is what
        // makes a writer's ordering persistent — with the name as a stable
        // tiebreak so equal indices do not shuffle between loads.
        std::sort(rows.begin(), rows.end(), [](const Entry& a, const Entry& b) {
            if (a.sortIndex != b.sortIndex) {
                return a.sortIndex < b.sortIndex;
            }
            return a.displayName.localeAwareCompare(b.displayName) < 0;
        });

        // ⚠️ THE RELATIONSHIP LABEL BELONGS HERE, not on every row (user ruling
        // 2026-08-30). A scene relates to its objects the same way each time —
        // every character "appears in" it — so a per-row label repeats one word
        // down the whole list while saying nothing that distinguishes the rows.
        //
        // ✅ "characters (2) (appears in)" says it ONCE, compacts the list, and
        // loses nothing.
        //
        // ⚠️ The labels are READ from the core's projection, never recomputed —
        // and they are collected from the ROWS rather than assumed, because a
        // group CAN legitimately hold more than one. Of the seeded vocabulary,
        // two types constrain to a scene: `appears-in` (scene "features" X) and
        // `located-at` (scene "takes place at" a location), so a Locations group
        // can hold both. ⚠️ When that happens ALL of them are named — narrowing
        // to the first would silently misdescribe the others.
        QStringList groupLabels;
        for (const Entry& e : rows) {
            if (!e.label.isEmpty() && !groupLabels.contains(e.label)) {
                groupLabels.append(e.label);
            }
        }
        auto* group = new QTreeWidgetItem(tree_);
        const QString countText = tr("%1 (%2)").arg(kind, QString::number(rows.size()));
        group->setText(0, groupLabels.isEmpty()
                              ? countText
                              : tr("%1 (%2)").arg(countText,
                                                  groupLabels.join(QStringLiteral(", "))));
        QFont groupFont = group->font(0);
        groupFont.setBold(true);
        group->setFont(0, groupFont);
        // A group row carries no objectID role, which is how the open paths tell
        // a heading from an object.
        group->setFlags(group->flags() & ~Qt::ItemIsSelectable);

        for (const Entry& e : rows) {
            auto* item = new QTreeWidgetItem(group);

            // ⚠️ THE ROW IS THE NAME ALONE. The relationship label lives on the
            // GROUP HEADER — see rebuildTree()'s header construction.
            //
            // ⚠️ User ruling 2026-08-30, and it corrects two of my mistakes:
            //
            //  1. ⚠️ "Myton at 23 — features" reads as though *Myton* features
            //     something. He does not. The stored edge is "Myton APPEARS IN
            //     scene"; asked from the SCENE's side the core projects the
            //     inverse, "scene FEATURES Myton". ⚠️ **The label describes what
            //     the SCENE does**, so rendering it beside the OBJECT's name
            //     inverts its meaning.
            //  2. ⚠️ Stacking it on a second line fixed the grammar and kept the
            //     REDUNDANCY: every row in a group repeats the identical word,
            //     because a scene relates to its objects the same way each time.
            //
            // ✅ Hoisting it to the header says it ONCE, compacts the list, and
            // loses nothing.
            const QString rowText = e.displayName;
            item->setText(0, rowText);
            // ⚠️ I-0173: the panel is ~200px by default, so a row of the form
            // "<name> — <relationship>" ELIDES to "character 1 — doc…" and the
            // relationship — the half only the core can supply — becomes
            // unreadable. A tooltip carries the full text so the information is
            // reachable without widening the panel or truncating the name.
            //
            // ⚠️ Deliberately NOT the only route: the panel is resizable and the
            // name (the part a writer scans by) is always the LEADING text, so
            // this is a supplement to a readable row, not a hover-only
            // affordance. Apple learned that one the hard way — T-0389 exists
            // because a pending object's world appeared ONLY in a tooltip.
            // ⚠️ The tooltip is the NAME only — the label is on the header, so
            // repeating it here would reintroduce exactly the duplication the
            // header hoist removed. The tooltip still earns its place: a long
            // name elides in a narrow panel (I-0173).
            item->setToolTip(0, rowText);
            item->setData(0, kRoleObjectID, e.objectID);
            item->setData(0, kRoleKind,     e.kind);
            item->setData(0, kRoleWorldID,  e.worldID);
            item->setData(0, kRolePending,  e.pending);
            item->setData(0, kRoleDisplayName, e.displayName);
            if (e.pending) {
                // ⚠️ Shown, named, and not modifiable — never hidden. The sentence
                // in the status line below names the world; this marks the row.
                item->setText(0, tr("⚠ %1").arg(e.displayName));
                // Both facts, not one replacing the other: a pending row still
                // needs its full label readable (I-0173).
                item->setToolTip(0, tr("%1\n\nHeld pending — this object's world is %2.")
                                        .arg(e.displayName, writerDescription(e.pendingStatus)));
            }
            ++shown;
        }
    }
    tree_->expandAll();

    // ---- The explained state (T-0483) -----------------------------------
    // Four conditions, four sentences. None of them is a blank panel.

    if (bridge_ == nullptr || projectRootPath_.isEmpty()) {
        setStatusLine(tr("No project open."), false);
        return;
    }
    if (!loadError_.isEmpty()) {
        setStatusLine(loadError_, true);
        return;
    }
    if (sceneID_.isEmpty()) {
        setStatusLine(tr("No scene selected."), false);
        return;
    }
    if (!ObjectKindScope::isLoaded(bridge_)) {
        // ⚠️ The kind table is the app's only source of kind scope. If the core
        // never answered, say so — an empty list here would read as "this project
        // has no objects", which is a different and false claim.
        setStatusLine(tr("Object kinds could not be read from ScriviCore."), true);
        return;
    }

    // Pending: one line per distinct world, so a scene holding entries from two
    // absent worlds reports both rather than blaming whichever sorted first.
    QHash<QString, QPair<QString, int>> pendingByWorld;   // worldID → (status, count)
    for (const Entry& e : entries_) {
        if (!e.pending) {
            continue;
        }
        auto& slot = pendingByWorld[e.worldID];
        if (slot.first.isEmpty()) {
            slot.first = e.pendingStatus;
        }
        slot.second += 1;
    }

    QStringList sentences;
    if (indexUnreadable_) {
        // ⚠️ Distinct from every other state here: the rows below came from the
        // graph alone, unconfirmed against the object index. Say so rather than
        // presenting them as verified — and rather than presenting an empty list
        // as an empty scene.
        sentences << tr("The object index could not be read; "
                        "this list is unconfirmed.");
    }
    if (shown == 0 && !indexUnreadable_) {
        // ⚠️ Genuinely empty — and said in a way that does not read as loss.
        sentences << tr("No objects are linked to this scene yet.");
    }
    QStringList pendingWorlds = pendingByWorld.keys();
    std::sort(pendingWorlds.begin(), pendingWorlds.end());
    for (const QString& worldID : pendingWorlds) {
        const QPair<QString, int> info = pendingByWorld.value(worldID);
        const QString name = worldDisplayName(worldID);
        const QString world = name.isEmpty() ? tr("That world")
                                             : tr("World “%1”").arg(name);
        const QString links = info.second == 1
                                  ? tr("This link is")
                                  : tr("These %1 links are").arg(info.second);
        // ⚠️ Never implies the links are gone — they are HELD, which is the
        // guarantee the writer most needs to know.
        sentences << tr("%1 is %2. %3 held pending.")
                         .arg(world, writerDescription(info.first), links);
    }

    setStatusLine(sentences.join(QStringLiteral(" ")), !pendingByWorld.isEmpty());
}

void SceneInspector::setStatusLine(const QString& text, bool warning)
{
    if (status_ == nullptr) {
        return;
    }
    status_->setText(text);
    status_->setVisible(!text.isEmpty());
    // Muted for an ordinary empty state; a warning colour when something is
    // actually wrong. The SENTENCE carries the meaning either way — colour alone
    // is never the signal.
    // ⚠️ I-0186: DERIVED, never a named palette role. `palette(link-visited)`
    // resolved to MAGENTA under both Yaru themes, and the muted branch's
    // `setEnabled(false)` below is itself theme-dependent. See ThemeColours.hpp.
    ThemeColours::applyTextColour(
        status_, warning ? ThemeColours::attention(status_->palette())
                         : ThemeColours::deemphasised(status_->palette()));
    // ⚠️ I-0186: previously `setEnabled(false)` was used to MUTE the ordinary
    // state. That routes the label through the palette's DISABLED group, which
    // overrides the colour set above and is itself theme-dependent — the same
    // class of bug. The colour above now carries the emphasis, so the widget
    // stays enabled and its text stays selectable.
    status_->setEnabled(true);
}

QString SceneInspector::writerDescription(const QString& statusName)
{
    // ⚠️ HOISTED to WorldStatusText (SP-127, T-0492) when the Worlds dialog became
    // a second reader. The wording lives in ONE place now; this stays only as the
    // call site the inspector already had.
    return WorldStatusText::writerDescription(statusName);
}

QString SceneInspector::worldDisplayName(const QString& worldID)
{
    if (worldID.isEmpty()) {
        return {};
    }
    // ⚠️ Falling back to the ID is deliberate: an unnamed world is still better
    // than an unattributed warning.
    const QString name = worldNames_.value(worldID);
    return name.isEmpty() ? worldID : name;
}

bool SceneInspector::eventFilter(QObject* watched, QEvent* event)
{
    // ⚠️ The ONLY reliable commit point for a QTextEdit. QLineEdit has
    // editingFinished; QTextEdit has nothing equivalent, so focus-out is where
    // an edit becomes a save.
    //
    // ⚠️ Deliberately NOT textChanged: that would write to disk on every
    // keystroke. And deliberately not only-on-scene-change: a writer who types a
    // note and clicks into the manuscript never changes scene, so the note would
    // sit in the widget and be discarded on the next load.
    if (event != nullptr && event->type() == QEvent::FocusOut) {
        if (watched == outlineEdit_) {
            onOutlineEdited();
        } else if (watched == todoEdit_) {
            onTodoEdited();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SceneInspector::loadSceneNotes()
{
    // ⚠️ ONE read feeds BOTH the Writing and Properties tabs — they are two
    // views of the same `getSceneNotes` result, so reading twice would be waste.
    loadingNotes_ = true;

    auto clearAll = [this](const QString& why) {
        if (tagsEdit_)     { tagsEdit_->clear(); }
        if (outlineEdit_)  { outlineEdit_->clear(); }
        if (todoEdit_)     { todoEdit_->clear(); }
        for (QLabel* l : {propTitle_, propWords_, propChars_, propCreated_, propModified_}) {
            if (l) { l->clear(); }
        }
        // ⚠️ Never leave both tabs blank and silent — the same rule T-0483
        // applies to the object list applies here.
        if (writingStatus_) { writingStatus_->setText(why); }
        if (propStatus_)    { propStatus_->setText(why); }
    };

    const bool haveContext = bridge_ != nullptr && !projectRootPath_.isEmpty();
    if (!haveContext) {
        clearAll(tr("No project open."));
        setWritingEnabled(false);
        loadingNotes_ = false;
        return;
    }
    if (sceneID_.isEmpty()) {
        clearAll(tr("No scene selected."));
        setWritingEnabled(false);
        loadingNotes_ = false;
        return;
    }

    const QVariantMap notes = bridge_->getSceneNotes(projectRootPath_, sceneID_);
    if (bridge_->lastCallFailed()) {
        // ⚠️ Distinguish "could not read" from "nothing written yet" — an empty
        // outline and an unreadable scene must never look the same. And DISABLE
        // editing: writing into fields we could not load would overwrite notes
        // we never saw.
        clearAll(tr("This scene's notes could not be read."));
        setWritingEnabled(false);
        loadingNotes_ = false;
        return;
    }
    setWritingEnabled(true);

    // --- Writing tab ------------------------------------------------------
    QStringList tags;
    for (const QVariant& t : notes.value(QStringLiteral("tags")).toList()) {
        tags << t.toString();
    }
    if (tagsEdit_) { tagsEdit_->setText(tags.join(QStringLiteral(", "))); }

    if (outlineEdit_) {
        outlineEdit_->setPlainText(notes.value(QStringLiteral("outline")).toString());
    }

    if (todoEdit_) {
        QStringList lines;
        for (const QVariant& v : notes.value(QStringLiteral("todo")).toList()) {
            const QVariantMap item = v.toMap();
            const QString text = item.value(QStringLiteral("text")).toString();
            lines << (item.value(QStringLiteral("done")).toBool()
                          ? QStringLiteral("[x] ") + text
                          : text);
        }
        todoEdit_->setPlainText(lines.join(QStringLiteral("\n")));
    }
    if (writingStatus_) { writingStatus_->clear(); }

    // --- Properties tab ---------------------------------------------------
    // ⚠️ DERIVED values only. Nothing here is editable, and nothing here is
    // recomputed in Qt — the counts are the core's.
    if (propTitle_)    { propTitle_->setText(notes.value(QStringLiteral("title")).toString()); }
    if (propWords_)    { propWords_->setText(QString::number(notes.value(QStringLiteral("wordCount")).toInt())); }
    if (propChars_)    { propChars_->setText(QString::number(notes.value(QStringLiteral("characterCount")).toInt())); }
    if (propCreated_) {
        const QString by = notes.value(QStringLiteral("createdByDisplayName")).toString();
        const QString at = notes.value(QStringLiteral("createdAt")).toString();
        propCreated_->setText(by.isEmpty() ? at : tr("%1 — %2").arg(at, by));
    }
    if (propModified_) {
        const QString by = notes.value(QStringLiteral("modifiedByDisplayName")).toString();
        const QString at = notes.value(QStringLiteral("modifiedAt")).toString();
        propModified_->setText(by.isEmpty() ? at : tr("%1 — %2").arg(at, by));
    }
    if (propStatus_) { propStatus_->clear(); }

    loadingNotes_ = false;
}

void SceneInspector::setWritingEnabled(bool on)
{
    // ⚠️ The Writing fields are genuinely disabled when the scene could not be
    // read — not merely styled. Typing into a field we failed to load would
    // save over notes we never saw.
    if (tagsEdit_)    { tagsEdit_->setEnabled(on); }
    if (outlineEdit_) { outlineEdit_->setReadOnly(!on); }
    if (todoEdit_)    { todoEdit_->setReadOnly(!on); }
}

void SceneInspector::flushPendingEdits()
{
    if (loadingNotes_ || bridge_ == nullptr || sceneID_.isEmpty()) {
        return;
    }
    // The two QTextEdits have no editingFinished signal, so their commit happens
    // here and on focus-out. Tags commit through their own signal.
    onOutlineEdited();
    onTodoEdited();
}

void SceneInspector::onTagsEdited()
{
    if (loadingNotes_ || bridge_ == nullptr || sceneID_.isEmpty() || tagsEdit_ == nullptr) {
        return;
    }
    // Comma-separated in the UI; an ARRAY on disk. ⚠️ The core owns the storage
    // shape — this is a presentation choice, and the split is explicit rather
    // than smuggled into the widget.
    QJsonArray arr;
    for (const QString& raw : tagsEdit_->text().split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const QString t = raw.trimmed();
        if (!t.isEmpty()) { arr.append(t); }
    }
    bridge_->setSceneTags(projectRootPath_, sceneID_,
                          QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

void SceneInspector::onOutlineEdited()
{
    if (loadingNotes_ || bridge_ == nullptr || sceneID_.isEmpty() || outlineEdit_ == nullptr) {
        return;
    }
    bridge_->setSceneOutline(projectRootPath_, sceneID_, outlineEdit_->toPlainText());
}

void SceneInspector::onTodoEdited()
{
    if (loadingNotes_ || bridge_ == nullptr || sceneID_.isEmpty() || todoEdit_ == nullptr) {
        return;
    }
    // One item per line; a leading "[x] " (or "[X] ") marks it done. ⚠️ A blank
    // line is DROPPED rather than stored as an empty todo — otherwise every
    // stray Return would accumulate an invisible item.
    QJsonArray arr;
    const QStringList lines = todoEdit_->toPlainText().split(QLatin1Char('\n'));
    for (const QString& raw : lines) {
        QString line = raw.trimmed();
        if (line.isEmpty()) { continue; }
        bool done = false;
        if (line.startsWith(QLatin1String("[x] "), Qt::CaseInsensitive)) {
            done = true;
            line = line.mid(4).trimmed();
        }
        if (line.isEmpty()) { continue; }
        QJsonObject item;
        item.insert(QStringLiteral("text"), line);
        item.insert(QStringLiteral("done"), done);
        arr.append(item);
    }
    bridge_->setSceneTodo(projectRootPath_, sceneID_,
                          QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

void SceneInspector::onItemActivated(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (item == nullptr) {
        return;
    }
    const QString objectID = item->data(0, kRoleObjectID).toString();
    if (objectID.isEmpty()) {
        // A kind heading. Toggling it is the useful thing a double-click can do
        // here, and it keeps the gesture from feeling dead.
        item->setExpanded(!item->isExpanded());
        return;
    }

    // ⚠️ A PENDING object cannot be opened — its world is away, so `openObject`
    // fails at the core and the writer gets a raw error naming a world UUID.
    //
    // ⚠️ THE CONTEXT MENU ALREADY GUARDED THIS AND DOUBLE-CLICK DID NOT — the two
    // halves of T-0482 disagreed, which is exactly the split
    // `feedback_verify_each_half_separately` warns about: I verified both
    // affordances OPENED an object and never verified both REFUSED one.
    // (Reported by the user 2026-08-30.)
    //
    // ✅ Explain in the panel, where the writer is already looking, rather than
    // firing a call we know will fail.
    if (item->data(0, kRolePending).toBool()) {
        const QString worldID = item->data(0, kRoleWorldID).toString();
        const QString name = worldDisplayName(worldID);
        setStatusLine(tr("“%1” can't be opened yet — %2 is unavailable. "
                         "Its links are held; nothing has been lost.")
                          .arg(item->data(0, kRoleDisplayName).toString(),
                               name.isEmpty() ? tr("its world")
                                              : tr("world “%1”").arg(name)),
                      true);
        return;
    }

    emit openObjectRequested(item->data(0, kRoleKind).toString(),
                             objectID,
                             item->data(0, kRoleWorldID).toString());
}

void SceneInspector::onContextMenuRequested(const QPoint& pos)
{
    if (tree_ == nullptr) {
        return;
    }
    QTreeWidgetItem* item = tree_->itemAt(pos);
    if (item == nullptr) {
        return;
    }
    const QString objectID = item->data(0, kRoleObjectID).toString();
    if (objectID.isEmpty()) {
        return;   // a kind heading has nothing to open
    }

    // ⚠️ T-0482: this is a SEPARATE route to the same outcome, and it is verified
    // separately. On Apple, AC7's two halves needed four Issues before the second
    // one worked (feedback_verify_each_half_separately).
    QMenu menu(tree_);
    QAction* open = menu.addAction(tr("Open Object"));
    const bool pending = item->data(0, kRolePending).toBool();
    if (pending) {
        // The graph is frozen toward an unavailable world. Disabled AND
        // EXPLAINED — never silently inert.
        open->setEnabled(false);
        open->setToolTip(tr("This object's world is unavailable."));
        menu.addSeparator();
        QAction* why = menu.addAction(tr("Held pending — its world is unavailable"));
        why->setEnabled(false);
    }
    // Select the row the menu was raised on, so the menu and the selection agree.
    tree_->setCurrentItem(item);

    QAction* chosen = menu.exec(tree_->viewport()->mapToGlobal(pos));
    if (chosen == open && open->isEnabled()) {
        emit openObjectRequested(item->data(0, kRoleKind).toString(),
                                 objectID,
                                 item->data(0, kRoleWorldID).toString());
    }
}
