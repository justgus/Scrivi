#include "SceneInspector.hpp"

#include "ObjectKindScope.hpp"
#include "ScriviBridge.hpp"

#include <QAction>
#include <QFont>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QSet>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

// The panel lives in EditorShell's QSplitter, so the minimum is a hard floor and
// the default is applied by the splitter's initial sizes. Narrower than Apple's
// 240/280 by user preference (2026-07-22): 120 min / 200 default keeps the writing
// surface wider.
namespace {
constexpr int kMinWidth     = 120;
constexpr int kDefaultWidth = 200;

// Roles carrying an entry's identity on its tree item. A kind GROUP row carries
// none of these, which is what distinguishes it from an object row.
constexpr int kRoleObjectID = Qt::UserRole + 1;
constexpr int kRoleKind     = Qt::UserRole + 2;
constexpr int kRoleWorldID  = Qt::UserRole + 3;
constexpr int kRolePending  = Qt::UserRole + 4;
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
    tabs_->addTab(buildSceneEntitiesTab(), tr("Scene Entities"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(tabs_);
}

QWidget* SceneInspector::buildSceneEntitiesTab()
{
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    title_ = new QLabel(tr("Scene Entities"), tab);
    QFont titleFont = title_->font();
    titleFont.setBold(true);
    title_->setFont(titleFont);
    title_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(title_);

    tree_ = new QTreeWidget(tab);
    tree_->setHeaderHidden(true);
    tree_->setColumnCount(1);
    tree_->setRootIsDecorated(true);
    tree_->setUniformRowHeights(true);
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    // T-0482: the context menu is a FIRST-CLASS path to open, not a convenience
    // duplicate of the double-click. The VNC input path carries no Shift-combos
    // and no trackpad gestures (project_linux_vnc_input_constraints), so every
    // action needs a button or menu route.
    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree_, &QTreeWidget::itemDoubleClicked,
            this, &SceneInspector::onItemActivated);
    // itemActivated covers Return/Enter on the keyboard — the same open, reachable
    // without a pointer at all.
    connect(tree_, &QTreeWidget::itemActivated,
            this, &SceneInspector::onItemActivated);
    connect(tree_, &QTreeWidget::customContextMenuRequested,
            this, &SceneInspector::onContextMenuRequested);
    layout->addWidget(tree_, 1);

    // ⚠️ T-0483: the panel's one explanatory line. A scene with no objects, an
    // unavailable world, a pending object and an unreachable core are DIFFERENT
    // states and each gets its own sentence here. It is never left blank while
    // the tree is empty.
    status_ = new QLabel(tab);
    status_->setWordWrap(true);
    status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(status_);

    // Nothing is known until setContext/setScene arrive; say so rather than
    // showing a bare empty list.
    setStatusLine(tr("No project open."), false);

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
    rebuildTree();
}

void SceneInspector::setScene(const QString& sceneID)
{
    if (sceneID == sceneID_) {
        return;   // scroll traffic re-reports the same scene constantly
    }
    sceneID_ = sceneID;
    reload();
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

        auto* group = new QTreeWidgetItem(tree_);
        group->setText(0, tr("%1 (%2)").arg(kind, QString::number(rows.size())));
        QFont groupFont = group->font(0);
        groupFont.setBold(true);
        group->setFont(0, groupFont);
        // A group row carries no objectID role, which is how the open paths tell
        // a heading from an object.
        group->setFlags(group->flags() & ~Qt::ItemIsSelectable);

        for (const Entry& e : rows) {
            auto* item = new QTreeWidgetItem(group);
            // The label is the core's, already resolved for this scene.
            const QString rowText = e.label.isEmpty()
                                        ? e.displayName
                                        : tr("%1 — %2").arg(e.displayName, e.label);
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
            item->setToolTip(0, rowText);
            item->setData(0, kRoleObjectID, e.objectID);
            item->setData(0, kRoleKind,     e.kind);
            item->setData(0, kRoleWorldID,  e.worldID);
            item->setData(0, kRolePending,  e.pending);
            if (e.pending) {
                // ⚠️ Shown, named, and not modifiable — never hidden. The sentence
                // in the status line below names the world; this marks the row.
                item->setText(0, tr("⚠ %1").arg(item->text(0)));
                // Both facts, not one replacing the other: a pending row still
                // needs its full label readable (I-0173).
                item->setToolTip(0, tr("%1\n\nHeld pending — this object's world is %2.")
                                        .arg(rowText, writerDescription(e.pendingStatus)));
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
    status_->setStyleSheet(warning ? QStringLiteral("color: palette(link-visited);")
                                   : QString());
    status_->setEnabled(!warning ? false : true);
}

QString SceneInspector::writerDescription(const QString& statusName)
{
    // Mirrors Apple's WorldStatus.writerDescription (ScriviError.swift:115) word
    // for word. ⚠️ "missing" is only ever reported when the CORE positively
    // established it; anything undetermined arrives as "unavailable" and must be
    // repeated as such, never upgraded.
    if (statusName == QLatin1String("available"))   { return tr("available"); }
    if (statusName == QLatin1String("offline"))     { return tr("offline"); }
    if (statusName == QLatin1String("unmounted"))   { return tr("on a disconnected volume"); }
    if (statusName == QLatin1String("missing"))     { return tr("missing"); }
    return tr("unavailable");
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
