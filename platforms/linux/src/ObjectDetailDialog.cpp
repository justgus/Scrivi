#include "ObjectDetailDialog.hpp"

#include "GlyphIcon.hpp"
#include "RelationTypes.hpp"

#include "AsyncCall.hpp"
#include "ScriviBridge.hpp"
#include "WorldStatusText.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeySequence>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QSet>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace {

constexpr int kContentPage = 0;
constexpr int kMessagePage = 1;
const QSize kPreviewBound(480, 220);   // Apple: scaled to fit, at most 220 high
// Apple's `SourcesCardModel.citesType` / `.sourceKind`: the citation edge, and the one project-scoped kind it comes from.
const QString kCites = QStringLiteral("cites");
const QString kSourceKind = QStringLiteral("source");

// I-0159: a list inside the dialog's own scroll area takes the height of its rows — one scroll region, never a capped
// inner one whose hidden rows read as work missing.
void fitToRows(QAbstractItemView* view, int rows)
{
    const int row = std::max(view->sizeHintForRow(0), view->fontMetrics().height() + 6);
    view->setFixedHeight(rows * row + 2 * view->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) + 2);
}

QString capitalised(const QString& s) { return s.isEmpty() ? s : s.left(1).toUpper() + s.mid(1); }

QPushButton* sectionButton(const QString& text, const char* name, QWidget* parent)
{
    auto* b = new QPushButton(text, parent);
    b->setObjectName(QString::fromLatin1(name));
    b->setAutoDefault(false);   // Return in a field must not press it (SP-235 live pass)
    b->setFlat(true);
    return b;
}

QLabel* warning(const char* name, QWidget* parent)
{
    auto* label = new QLabel(parent);
    label->setObjectName(QString::fromLatin1(name));
    label->setWordWrap(true);
    label->setStyleSheet(QStringLiteral("QLabel { color: #b35c00; }"));
    label->hide();
    return label;
}

// A refusal toward an absent world, in the core's words (the same match as `SceneInspector::runObjectWrite` and
// `EditorShell::writerFacingError`): fail-safe — an unmatched message is reported as a failure, never hidden.
bool isWorldAwayMessage(const QString& message)
{
    return message.contains(QLatin1String("worldUnavailable")) || message.contains(QLatin1String("is unavailable"))
        || message.contains(QLatin1String("is offline"));
}

// Tags round-trip as `[{"v": …}]` (`ObjectJson.cpp`), and an empty list is written as no key at all (Apple:
// `ObjectDetail.applyingEdits`).
QStringList tagsOf(const QJsonObject& json)
{
    QStringList tags;
    for (const QJsonValue& v : json.value(QStringLiteral("tags")).toArray()) {
        const QString t = v.toObject().value(QStringLiteral("v")).toString();
        if (!t.isEmpty()) { tags << t; }
    }
    return tags;
}

// Patch, never rebuild: `image`, `attributes` and every key a later core adds survive.
QJsonObject patched(QJsonObject json, const QString& name, const QString& subtitle, const QString& notes,
                    const QStringList& tags)
{
    json.insert(QStringLiteral("displayName"), name.trimmed());
    json.insert(QStringLiteral("subtitle"), subtitle);
    json.insert(QStringLiteral("notes"), notes);
    QJsonArray out;
    for (const QString& t : tags) {
        const QString clean = t.trimmed();
        if (!clean.isEmpty()) { out.append(QJsonObject{{QStringLiteral("v"), clean}}); }
    }
    if (out.isEmpty()) {
        json.remove(QStringLiteral("tags"));
    } else {
        json.insert(QStringLiteral("tags"), out);
    }
    return json;
}

// Set or clear `image.assetID` on the object as it is on disk now (Apple: `ObjectImageSection.attachImage`).
QString attachImage(ScriviBridge& b, const QString& root, const QString& kind, const QString& objectID,
                    const QString& worldID, const QString& assetID)
{
    const QVariantMap opened = b.openObject(root, kind, objectID, worldID);
    QJsonObject json = QJsonDocument::fromJson(opened.value(QStringLiteral("objectJson")).toString().toUtf8()).object();
    if (b.lastCallFailed() || json.isEmpty()) { return QObject::tr("This object could not be read."); }
    if (assetID.isEmpty()) {
        json.remove(QStringLiteral("image"));
    } else {
        QJsonObject image = json.value(QStringLiteral("image")).toObject();
        image.insert(QStringLiteral("assetID"), assetID);
        json.insert(QStringLiteral("image"), image);
    }
    b.saveObject(root, kind, QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
    return b.lastCallFailed() ? QObject::tr("The image could not be saved to this object.") : QString();
}

// Delete an asset only when no other object still shows it: Choose Existing… lets two objects share one image, and deleting
// a shared one would take the bytes from under the other ([I-0369], where the Mac deletes on Replace and Remove regardless).
void removeAssetIfUnshared(ScriviBridge& b, const QString& root, const QString& assetID, const QString& assetPath,
                           const QString& objectID, const QString& worldID, const QString& projectID)
{
    if (assetID.isEmpty() || assetPath.isEmpty()) { return; }   // cannot tell who else uses it: keep the bytes
    for (const QVariant& v : b.listObjects(root, QString()).value(QStringLiteral("objects")).toList()) {
        const QVariantMap m = v.toMap();
        if (m.value(QStringLiteral("objectID")).toString() != objectID
            && m.value(QStringLiteral("imagePath")).toString() == assetPath) {
            return;
        }
    }
    if (b.lastCallFailed()) { return; }
    b.removeAsset(root, assetID, worldID, projectID);
}

// Tags wrap like Apple's `FlowLayout`; Qt ships none (only an example), so this is the minimum of one.
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget* parent) : QLayout(parent) { setContentsMargins(0, 0, 0, 0); }
    ~FlowLayout() override { while (QLayoutItem* item = takeAt(0)) { delete item; } }
    void addItem(QLayoutItem* item) override { items_.append(item); }
    int count() const override { return int(items_.size()); }
    QLayoutItem* itemAt(int i) const override { return items_.value(i); }
    QLayoutItem* takeAt(int i) override { return (i >= 0 && i < items_.size()) ? items_.takeAt(i) : nullptr; }
    Qt::Orientations expandingDirections() const override { return {}; }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override { return arrange(QRect(0, 0, width, 0), false); }
    QSize sizeHint() const override { return minimumSize(); }
    QSize minimumSize() const override
    {
        QSize size;
        for (const QLayoutItem* item : items_) { size = size.expandedTo(item->minimumSize()); }
        return size;
    }
    void setGeometry(const QRect& rect) override
    {
        QLayout::setGeometry(rect);
        arrange(rect, true);
    }

private:
    int arrange(const QRect& rect, bool apply) const
    {
        constexpr int kGap = 6;
        int x = rect.x();
        int y = rect.y();
        int line = 0;
        for (QLayoutItem* item : items_) {
            const QSize hint = item->sizeHint();
            if (x + hint.width() > rect.right() && line > 0) {
                x = rect.x();
                y += line + kGap;
                line = 0;
            }
            if (apply) { item->setGeometry(QRect(QPoint(x, y), hint)); }
            x += hint.width() + kGap;
            line = std::max(line, hint.height());
        }
        return y + line - rect.y();
    }
    QList<QLayoutItem*> items_;
};

QLabel* caption(const QString& text, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    QFont f = label->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 0.9);
    label->setFont(f);
    label->setForegroundRole(QPalette::PlaceholderText);
    return label;
}

QLabel* banner(const QString& name, QWidget* parent)
{
    auto* label = new QLabel(parent);
    label->setObjectName(name);
    label->setWordWrap(true);
    label->setFrameShape(QFrame::StyledPanel);
    label->setMargin(8);
    label->hide();
    return label;
}

}  // namespace

ObjectDetailDialog::ObjectDetailDialog(ScriviBridge* bridge, const QString& projectRoot, const QString& projectID,
                                       const Entry& entry, QWidget* parent)
    : QDialog(parent), bridge_(bridge), root_(projectRoot), projectID_(projectID), entry_(entry)
{
    setObjectName(QStringLiteral("objectDetail"));
    setWindowModality(Qt::WindowModal);   // Q1: over the project window, as Apple's sheet
    setMinimumSize(520, 520);
    resize(620, 680);
    setWindowTitle(entry_.displayName.isEmpty() ? tr("Object") : entry_.displayName);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // Q2: Save and Undo Changes beside Close, trailing; the theme's standard icons, the words on tooltips.
    auto* bar = new QToolBar(this);
    bar->setObjectName(QStringLiteral("detailToolbar"));
    bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    // C3: Back at the leading end, as the platform's chevron (Apple's `NavigationStack`); shown once there is somewhere to go.
    backAction_ = bar->addAction(QIcon::fromTheme(QStringLiteral("go-previous"), style()->standardIcon(QStyle::SP_ArrowLeft)),
                                 tr("Back"));
    backAction_->setObjectName(QStringLiteral("detailBack"));
    backAction_->setShortcut(QKeySequence::Back);
    backAction_->setVisible(false);
    addAction(backAction_);
    connect(backAction_, &QAction::triggered, this, &ObjectDetailDialog::goBack);
    auto* spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bar->addWidget(spacer);
    saveAction_ = bar->addAction(QIcon::fromTheme(QStringLiteral("document-save"),
                                                  style()->standardIcon(QStyle::SP_DialogSaveButton)),
                                 tr("Save Changes"));
    saveAction_->setObjectName(QStringLiteral("detailSave"));
    saveAction_->setShortcut(QKeySequence::Save);
    saveAction_->setToolTip(tr("Save Changes (Ctrl+S)"));
    revertAction_ = bar->addAction(QIcon::fromTheme(QStringLiteral("edit-undo"),
                                                    style()->standardIcon(QStyle::SP_ArrowBack)),
                                   tr("Undo Changes"));
    revertAction_->setObjectName(QStringLiteral("detailRevert"));
    revertAction_->setToolTip(tr("Undo Changes — discard your edits and return to the saved version"));
    // A9: ⋯ after Undo Changes, before Close — the Mac's order ([I-0371]). Filled as it opens, so it always reads the state.
    more_ = new QToolButton(bar);
    more_->setObjectName(QStringLiteral("detailMore"));
    more_->setText(QStringLiteral("⋯"));   // shown when the theme has no `view-more` (Qt has no standard "more" icon)
    more_->setIcon(GlyphIcon::icon(QStringLiteral("inkwyn.more.options")));   // EP-060 SP-243 Q7
    more_->setToolTip(tr("More"));
    more_->setPopupMode(QToolButton::InstantPopup);
    more_->setMenu(new QMenu(more_));
    more_->menu()->setObjectName(QStringLiteral("detailMoreMenu"));
    connect(more_->menu(), &QMenu::aboutToShow, this, &ObjectDetailDialog::fillMoreMenu);
    bar->addWidget(more_);
    closeAction_ = bar->addAction(GlyphIcon::icon(QStringLiteral("inkwyn.close")), tr("Close"));
    closeAction_->setObjectName(QStringLiteral("detailClose"));
    closeAction_->setToolTip(tr("Close the detail view (Esc)"));
    addAction(saveAction_);   // Ctrl+S anywhere in the dialog
    connect(saveAction_, &QAction::triggered, this, &ObjectDetailDialog::save);
    connect(revertAction_, &QAction::triggered, this, &ObjectDetailDialog::revert);
    connect(closeAction_, &QAction::triggered, this, &ObjectDetailDialog::requestClose);
    outer->addWidget(bar);

    pages_ = new QStackedWidget(this);
    outer->addWidget(pages_, 1);

    // ── The content page ────────────────────────────────────────────────────────────────────────────────────
    auto* scroll = new QScrollArea(pages_);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget(scroll);
    auto* col = new QVBoxLayout(content);
    col->setContentsMargins(12, 12, 12, 12);
    col->setSpacing(10);

    readOnlyBanner_ = banner(QStringLiteral("detailReadOnly"), content);
    atRiskBanner_ = banner(QStringLiteral("detailAtRisk"), content);
    atRiskBanner_->setText(tr("Your unsaved changes are held here only. They will be saved when this object's world is "
                              "available again — but they will be lost if Inkwyn quits first."));
    atRiskBanner_->setStyleSheet(QStringLiteral("QLabel { color: #b35c00; }"));
    col->addWidget(readOnlyBanner_);
    col->addWidget(atRiskBanner_);

    col->addWidget(caption(tr("Name"), content));
    name_ = new QLineEdit(content);
    name_->setObjectName(QStringLiteral("detailName"));
    name_->setPlaceholderText(tr("Name"));
    col->addWidget(name_);

    col->addWidget(caption(tr("Subtitle"), content));
    subtitle_ = new QLineEdit(content);
    subtitle_->setObjectName(QStringLiteral("detailSubtitle"));
    subtitle_->setPlaceholderText(tr("A one-line descriptor"));
    col->addWidget(subtitle_);

    col->addWidget(caption(tr("Notes"), content));
    notes_ = new QPlainTextEdit(content);
    notes_->setObjectName(QStringLiteral("detailNotes"));
    notes_->setMinimumHeight(160);
    col->addWidget(notes_);

    // Q3: chips with ✕ and an "Add a tag…" field, as Apple's.
    col->addWidget(caption(tr("Tags"), content));
    noTags_ = new QLabel(content);
    noTags_->setObjectName(QStringLiteral("detailNoTags"));
    noTags_->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(noTags_);
    tagChips_ = new QWidget(content);
    tagChips_->setObjectName(QStringLiteral("detailTags"));
    new FlowLayout(tagChips_);
    col->addWidget(tagChips_);
    newTag_ = new QLineEdit(content);
    newTag_->setObjectName(QStringLiteral("detailNewTag"));
    newTag_->setPlaceholderText(tr("Add a tag…"));
    connect(newTag_, &QLineEdit::returnPressed, this, &ObjectDetailDialog::addTag);
    col->addWidget(newTag_);

    saveError_ = new QLabel(content);
    saveError_->setObjectName(QStringLiteral("detailSaveError"));
    saveError_->setWordWrap(true);
    saveError_->setStyleSheet(QStringLiteral("QLabel { color: #b35c00; }"));
    saveError_->hide();
    col->addWidget(saveError_);

    auto* rule = new QFrame(content);
    rule->setFrameShape(QFrame::HLine);
    rule->setFrameShadow(QFrame::Sunken);
    col->addWidget(rule);

    // B2: the image, above anything the object is connected to (Apple: what the object IS comes first).
    col->addWidget(caption(tr("Image"), content));
    imagePreview_ = new QLabel(content);
    imagePreview_->setObjectName(QStringLiteral("detailImage"));
    imagePreview_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    col->addWidget(imagePreview_);
    imageMessage_ = new QLabel(content);
    imageMessage_->setObjectName(QStringLiteral("detailImageMessage"));
    imageMessage_->setWordWrap(true);
    imageMessage_->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(imageMessage_);
    imageFile_ = new QLabel(content);
    imageFile_->setObjectName(QStringLiteral("detailImageFile"));
    imageFile_->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(imageFile_);
    auto* imageRow = new QHBoxLayout;
    imageAdd_ = new QPushButton(content);
    imageAdd_->setObjectName(QStringLiteral("detailImageAdd"));
    imageExisting_ = new QPushButton(tr("Choose Existing…"), content);
    imageExisting_->setObjectName(QStringLiteral("detailImageExisting"));
    imageRemove_ = new QPushButton(tr("Remove"), content);
    imageRemove_->setObjectName(QStringLiteral("detailImageRemove"));
    imageRow->addWidget(imageAdd_);
    imageRow->addWidget(imageExisting_);
    imageRow->addWidget(imageRemove_);
    imageRow->addStretch(1);
    // A QDialog makes its first push button the default, so Return in any field would press Add Image… (SP-235 live pass).
    for (QPushButton* b : {imageAdd_, imageExisting_, imageRemove_}) { b->setAutoDefault(false); }
    col->addLayout(imageRow);
    connect(imageAdd_, &QPushButton::clicked, this, &ObjectDetailDialog::chooseImageFile);
    connect(imageExisting_, &QPushButton::clicked, this, &ObjectDetailDialog::chooseExisting);
    connect(imageRemove_, &QPushButton::clicked, this, &ObjectDetailDialog::removeImage);
    imageError_ = new QLabel(content);
    imageError_->setObjectName(QStringLiteral("detailImageError"));
    imageError_->setWordWrap(true);
    imageError_->setStyleSheet(QStringLiteral("QLabel { color: #b35c00; }"));
    imageError_->hide();
    col->addWidget(imageError_);

    auto divider = [content, col] {
        auto* r = new QFrame(content);
        r->setFrameShape(QFrame::HLine);
        r->setFrameShadow(QFrame::Sunken);
        col->addWidget(r);
    };

    // C4: Sources — between what the object IS and what it is connected to (Apple: `ObjectSourcesSection`, its own section,
    // not a row among the relationships, because a citation is attribution).
    divider();
    auto* sourcesHead = new QHBoxLayout;
    sourcesHead->addWidget(caption(tr("Sources"), content));
    sourcesHead->addStretch(1);
    cite_ = sectionButton(tr("Cite…"), "detailCite", content);
    newSource_ = sectionButton(tr("New Source…"), "detailNewSource", content);
    sourcesHead->addWidget(cite_);
    sourcesHead->addWidget(newSource_);
    col->addLayout(sourcesHead);
    connect(cite_, &QPushButton::clicked, this, &ObjectDetailDialog::chooseSource);
    connect(newSource_, &QPushButton::clicked, this, &ObjectDetailDialog::newSource);
    sourcesError_ = warning("detailSourcesError", content);
    col->addWidget(sourcesError_);
    noSources_ = new QLabel(content);
    noSources_->setObjectName(QStringLiteral("detailNoSources"));
    noSources_->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(noSources_);
    sources_ = new QListWidget(content);
    sources_->setObjectName(QStringLiteral("detailSources"));
    sources_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sources_->setContextMenuPolicy(Qt::CustomContextMenu);
    col->addWidget(sources_);
    sourceClick_ = new QTimer(this);
    sourceClick_->setSingleShot(true);
    sourceClick_->setInterval(QApplication::doubleClickInterval());
    // A click shows the citation; a double-click (or Return) opens the source. The click waits out the double-click
    // interval so one gesture does one thing.
    connect(sources_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        sourceClickEdge_ = item->data(Qt::UserRole).toString();
        sourceClick_->start();
    });
    connect(sourceClick_, &QTimer::timeout, this, [this] {
        if (const Edge* e = edgeFor(sourceClickEdge_)) { citeOf(*e); }
    });
    connect(sources_, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        sourceClick_->stop();
        if (const Edge* e = edgeFor(item->data(Qt::UserRole).toString())) { activateEdge(*e); }
    });
    connect(sources_, &QListWidget::customContextMenuRequested, this, &ObjectDetailDialog::sourceMenu);

    // C1: Related (Apple: `ObjectRelationsSection`).
    divider();
    auto* relatedHead = new QHBoxLayout;
    relatedHead->addWidget(caption(tr("Related"), content));
    relatedHead->addStretch(1);
    relate_ = sectionButton(tr("Relate…"), "detailRelate", content);
    relatedHead->addWidget(relate_);
    col->addLayout(relatedHead);
    connect(relate_, &QPushButton::clicked, this, [this] { chooseRelation(); });
    relationsError_ = warning("detailRelationsError", content);
    col->addWidget(relationsError_);
    noRelations_ = new QLabel(tr("Not related to anything yet."), content);
    noRelations_->setObjectName(QStringLiteral("detailNoRelations"));
    noRelations_->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(noRelations_);
    relations_ = new QTreeWidget(content);
    relations_->setObjectName(QStringLiteral("detailRelations"));
    relations_->setColumnCount(2);
    relations_->setHeaderHidden(true);
    relations_->setRootIsDecorated(false);
    relations_->setUniformRowHeights(true);
    relations_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    relations_->setContextMenuPolicy(Qt::CustomContextMenu);
    relations_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    relations_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    relations_->header()->setStretchLastSection(false);
    col->addWidget(relations_);
    connect(relations_, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* item) {
        if (const Edge* e = edgeFor(item->data(0, Qt::UserRole).toString())) { activateEdge(*e); }
    });
    connect(relations_, &QTreeWidget::customContextMenuRequested, this, &ObjectDetailDialog::relationMenu);

    divider();
    metadata_ = new QLabel(content);
    metadata_->setObjectName(QStringLiteral("detailMetadata"));
    metadata_->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(metadata_);
    col->addStretch(1);
    scroll->setWidget(content);
    pages_->insertWidget(kContentPage, scroll);

    // ── The message page: loading, a world away at open (I-0166), or a real failure ─────────────────────────
    messagePage_ = new QWidget(pages_);
    auto* msg = new QVBoxLayout(messagePage_);
    msg->addStretch(1);
    messageTitle_ = new QLabel(messagePage_);
    messageTitle_->setObjectName(QStringLiteral("detailMessageTitle"));
    QFont tf = messageTitle_->font();
    tf.setBold(true);
    messageTitle_->setFont(tf);
    messageText_ = new QLabel(messagePage_);
    messageText_->setObjectName(QStringLiteral("detailMessage"));
    messageHint_ = new QLabel(messagePage_);
    messageHint_->setObjectName(QStringLiteral("detailMessageHint"));
    messageHint_->setForegroundRole(QPalette::PlaceholderText);
    for (QLabel* l : {messageTitle_, messageText_, messageHint_}) {
        l->setAlignment(Qt::AlignCenter);
        l->setWordWrap(true);
        msg->addWidget(l);
    }
    msg->addStretch(1);
    pages_->insertWidget(kMessagePage, messagePage_);
    messageTitle_->setText(entry_.displayName);
    messageText_->setText(tr("Loading…"));
    messageHint_->clear();
    pages_->setCurrentIndex(kMessagePage);

    // ── The status line: what just happened, since Save and Undo Changes keep the dialog open (I-0246) ───────
    status_ = new QLabel(this);
    status_->setObjectName(QStringLiteral("detailStatus"));
    status_->setContentsMargins(12, 6, 12, 6);
    status_->setForegroundRole(QPalette::PlaceholderText);
    status_->hide();
    outer->addWidget(status_);

    for (QLineEdit* e : {name_, subtitle_}) {
        connect(e, &QLineEdit::textChanged, this, &ObjectDetailDialog::refreshState);
    }
    connect(notes_, &QPlainTextEdit::textChanged, this, &ObjectDetailDialog::refreshState);

    refreshState();
    reload();
}

ObjectDetailDialog::Detail ObjectDetailDialog::parse(const QJsonObject& json)
{
    Detail d;
    d.json = json;
    d.displayName = json.value(QStringLiteral("displayName")).toString();
    d.subtitle = json.value(QStringLiteral("subtitle")).toString();
    d.notes = json.value(QStringLiteral("notes")).toString();
    d.modifiedAt = json.value(QStringLiteral("modifiedAt")).toString();
    d.imageAssetID = json.value(QStringLiteral("image")).toObject().value(QStringLiteral("assetID")).toString();
    d.tags = tagsOf(json);
    return d;
}

void ObjectDetailDialog::reload()
{
    if (bridge_ == nullptr) { return; }
    const int generation = ++readGeneration_;
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_;
    const Entry entry = entry_;
    AsyncCall::run<Read>(
        this,
        [handle, root, entry]() -> Read {
            WorkerBridge bridge(handle);
            Read r;
            bool away = false;
            QString lastError;
            QObject::connect(&*bridge, &ScriviBridge::errorOccurred, [&away, &lastError](int, const QString& message) {
                lastError = message;
                if (isWorldAwayMessage(message)) { away = true; }
            });
            const QVariantList worlds = bridge->listWorlds(root).value(QStringLiteral("worlds")).toList();
            if (!bridge->lastCallFailed()) {
                r.worldsRead = true;
                for (const QVariant& v : worlds) {
                    const QVariantMap m = v.toMap();
                    World w;
                    w.name = m.value(QStringLiteral("displayName")).toString();
                    w.status = m.value(QStringLiteral("status")).toString();
                    w.reason = m.value(QStringLiteral("statusReason")).toString();
                    r.worlds.insert(m.value(QStringLiteral("worldID")).toString(), w);
                }
            }
            if (!entry.worldID.isEmpty()) {
                // ⚠️ Never call into a world known to be away: the read would wait on a dead volume (I-0193).
                const auto it = r.worlds.constFind(entry.worldID);
                const bool knownAway = r.worldsRead ? (it == r.worlds.constEnd() || !WorldStatusText::isAvailable(it->status))
                                                    : entry.pending;
                if (knownAway) {
                    r.worldAway = true;
                    return r;
                }
            }
            const QVariantMap opened = bridge->openObject(root, entry.kind, entry.objectID, entry.worldID);
            r.json = QJsonDocument::fromJson(opened.value(QStringLiteral("objectJson")).toString().toUtf8()).object();
            if (bridge->lastCallFailed() || r.json.isEmpty()) {
                r.worldAway = away;
                r.error = away ? QString() : (lastError.isEmpty() ? QObject::tr("This object could not be read.") : lastError);
                return r;
            }
            r.opened = true;
            // C1/C4: one read of the graph from THIS end — each edge's label is the core's, projected for this endpoint.
            const QVariantList edges = bridge->listEdgesFor(root, entry.objectID).value(QStringLiteral("edges")).toList();
            r.edgesRead = !bridge->lastCallFailed();
            for (const QVariant& v : edges) {
                const QVariantMap m = v.toMap();
                Edge e;
                e.edgeID = m.value(QStringLiteral("edgeID")).toString();
                e.relationType = m.value(QStringLiteral("relationType")).toString();
                e.label = m.value(QStringLiteral("label")).toString();
                e.otherID = m.value(QStringLiteral("otherID")).toString();
                e.otherName = m.value(QStringLiteral("otherDisplayName")).toString();
                e.otherKind = m.value(QStringLiteral("otherKind")).toString();   // I-0124: on the edge, so a pending one is named
                e.otherWorldID = m.value(QStringLiteral("otherWorldID")).toString();
                e.otherWorldStatus = m.value(QStringLiteral("otherWorldStatus")).toString();
                e.pending = m.value(QStringLiteral("otherPending")).toBool();
                r.edges.append(e);
            }
            // The path is the CORE's, from the listing (T-0446) — never a walk of the assets directory here.
            for (const QVariant& v : bridge->listObjects(root, entry.kind).value(QStringLiteral("objects")).toList()) {
                const QVariantMap m = v.toMap();
                if (m.value(QStringLiteral("objectID")).toString() == entry.objectID) {
                    r.imagePath = m.value(QStringLiteral("imagePath")).toString();
                    break;
                }
            }
            if (!r.imagePath.isEmpty() && QFile::exists(r.imagePath)) {
                QImage image(r.imagePath);
                if (!image.isNull()) {
                    r.image = (image.width() > kPreviewBound.width() || image.height() > kPreviewBound.height())
                                  ? image.scaled(kPreviewBound, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                                  : image;
                }
            }
            return r;
        },
        [this, generation](const Read& read) {
            if (generation == readGeneration_) { applyRead(read); }
        },
        [this, generation] {
            if (generation != readGeneration_ || hasDetail_) { return; }   // keep what we have
            messageText_->setText(tr("This object is taking longer than expected to read — its world may be on a slow or "
                                     "unreachable volume. Nothing has been lost."));
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::applyRead(const Read& read)
{
    if (read.worldsRead) {
        worlds_ = read.worlds;
        worldsKnown_ = true;
    }
    if (read.opened) {
        awayLatched_ = false;
        const Detail parsed = parse(read.json);
        // I-0165b: adopt disk only for fields the writer has not edited — and only for the SAME object.
        const bool same = hasDetail_;
        {
            const QSignalBlocker b1(name_), b2(subtitle_), b3(notes_);
            if (!same || name_->text() == detail_.displayName) { name_->setText(parsed.displayName); }
            if (!same || subtitle_->text() == detail_.subtitle) { subtitle_->setText(parsed.subtitle); }
            if (!same || notes_->toPlainText() == detail_.notes) { notes_->setPlainText(parsed.notes); }
            if (!same || tags_ == detail_.tags) { tags_ = parsed.tags; }
        }
        detail_ = parsed;
        hasDetail_ = true;
        imagePath_ = read.imagePath;
        image_ = read.image;
        edges_ = read.edges;
        edgesRead_ = read.edgesRead;
        pages_->setCurrentIndex(kContentPage);
    } else if (hasDetail_) {
        // I-0165: losing the ability to re-read is not having nothing to show. Keep the fields; the banner says why.
        if (read.worldAway) { awayLatched_ = true; }
        imagePath_.clear();   // the bytes are unreachable; stop pointing at them
        image_ = QImage();
    } else if (read.worldAway) {
        // I-0166: named and explained, never an error code.
        const QString world = worldName();
        const auto it = worlds_.constFind(entry_.worldID);
        const QString desc = WorldStatusText::writerDescription(it == worlds_.constEnd() ? QString() : it->status);
        messageTitle_->setText(entry_.displayName.isEmpty() ? tr("This object") : entry_.displayName);
        messageText_->setText(world.isEmpty() ? tr("This object's world is %1.").arg(desc)
                                              : tr("“%1” is %2.").arg(world, desc));
        messageHint_->setText(tr("Its details will appear when the world is available again."));
        pages_->setCurrentIndex(kMessagePage);
    } else {
        messageTitle_->setText(entry_.displayName);
        messageText_->setText(read.error);
        messageHint_->clear();
        pages_->setCurrentIndex(kMessagePage);
    }
    rebuildTags();
    rebuildGraph();
    refreshState();
}

bool ObjectDetailDialog::isReadOnly() const
{
    if (entry_.worldID.isEmpty()) { return false; }
    if (awayLatched_) { return true; }
    if (!worldsKnown_) { return false; }
    const auto it = worlds_.constFind(entry_.worldID);
    return it == worlds_.constEnd() || !WorldStatusText::isAvailable(it->status);   // a world this project cannot see, too
}

bool ObjectDetailDialog::hasChanges() const
{
    return hasDetail_
        && (name_->text() != detail_.displayName || subtitle_->text() != detail_.subtitle
            || notes_->toPlainText() != detail_.notes || tags_ != detail_.tags);
}

QString ObjectDetailDialog::worldName() const
{
    const auto it = worlds_.constFind(entry_.worldID);
    return (it == worlds_.constEnd() || it->name.isEmpty()) ? QString() : it->name;
}

QString ObjectDetailDialog::readOnlyBanner() const
{
    const auto it = worlds_.constFind(entry_.worldID);
    if (worldsKnown_ && it == worlds_.constEnd()) {
        return tr("This object belongs to a world this project cannot find, so it cannot be edited here.");
    }
    if (it != worlds_.constEnd() && it->reason == QLatin1String("unsupportedWorldFormatVersion")) {   // T-0440, Apple's words
        return tr("This world was saved by a newer version of Inkwyn. Update Inkwyn to open it. Your world has not been "
                  "changed.");
    }
    const QString status = (it != worlds_.constEnd() && !WorldStatusText::isAvailable(it->status)) ? it->status : QString();
    const QString name = worldName();
    return tr("“%1” is %2, so this object is read-only until it is available again.")
        .arg(name.isEmpty() ? tr("This object's world") : name, WorldStatusText::writerDescription(status));
}

void ObjectDetailDialog::setStatus(const QString& text)
{
    status_->setText(text);
    status_->setVisible(!text.isEmpty());
}

void ObjectDetailDialog::refreshState()
{
    const bool readOnly = isReadOnly();
    const bool changed = hasChanges();
    if (changed) { setStatus({}); }   // I-0246: the status never describes a state the writer has typed past

    if (hasDetail_) {
        setWindowTitle(detail_.displayName.isEmpty() ? tr("Object") : detail_.displayName);
    }
    readOnlyBanner_->setVisible(readOnly && hasDetail_);
    if (readOnly) { readOnlyBanner_->setText(readOnlyBanner()); }
    atRiskBanner_->setVisible(readOnly && changed);   // T-0450: only when edits are actually held
    name_->setReadOnly(readOnly);
    subtitle_->setReadOnly(readOnly);
    notes_->setReadOnly(readOnly);
    notes_->setPlaceholderText(readOnly ? tr("No notes.") : QString());
    newTag_->setVisible(!readOnly);

    saveAction_->setEnabled(!saving_ && changed && !readOnly);
    revertAction_->setEnabled(!saving_ && changed);
    more_->setEnabled(hasDetail_ && !saving_ && !readOnly);   // greyed while the world is away (the Mac's rule)

    // The image (Apple: `ObjectImageSection`).
    const bool hasImage = !detail_.imageAssetID.isEmpty();
    imagePreview_->clear();
    imagePreview_->hide();
    imageFile_->hide();
    if (!imagePath_.isEmpty() && QFileInfo::exists(imagePath_) && !image_.isNull()) {
        imagePreview_->setPixmap(QPixmap::fromImage(image_));
        imagePreview_->show();
        imageFile_->setText(QFileInfo(imagePath_).fileName());
        imageFile_->show();
        imageMessage_->hide();
    } else {
        imageMessage_->setText(!imagePath_.isEmpty() && QFileInfo::exists(imagePath_)
                                   ? tr("This image could not be read from disk.")
                               : hasImage ? tr("This object's image is in a world that is unavailable.")
                                          : tr("No image."));
        imageMessage_->show();
    }
    const QString lockedWhy = tr("This object's world is unavailable, so its image cannot be changed.");
    imageAdd_->setText(hasImage ? tr("Replace…") : tr("Add Image…"));
    imageAdd_->setEnabled(hasDetail_ && !readOnly && !imageWorking_);
    imageAdd_->setToolTip(readOnly ? lockedWhy
                                   : (hasImage ? tr("Choose a different image") : tr("Choose an image for this object")));
    imageExisting_->setEnabled(hasDetail_ && !readOnly && !imageWorking_);
    imageExisting_->setToolTip(readOnly ? lockedWhy : tr("Use an image already in this world"));
    imageRemove_->setVisible(hasImage);
    imageRemove_->setEnabled(!readOnly && !imageWorking_);
    imageRemove_->setToolTip(readOnly ? tr("This object's world is unavailable, so its image cannot be removed.")
                                      : tr("Remove this image from the object"));

    // C1/C4: the graph is frozen toward an unavailable world in both directions (Doc 3 §4.6 rule 1).
    const QString frozen = tr("This object's world is unavailable, so its relationships are frozen.");
    const QString citationsFrozen = tr("This object's world is unavailable, so its citations are frozen.");
    for (QPushButton* b : {cite_, newSource_, relate_}) { b->setEnabled(hasDetail_ && !readOnly && !graphWorking_); }
    cite_->setToolTip(readOnly ? citationsFrozen : tr("Cite a source that already exists in this project"));
    newSource_->setToolTip(readOnly ? citationsFrozen
                                    : tr("Create a source documenting “%1”").arg(detail_.displayName));
    relate_->setToolTip(readOnly ? frozen : tr("Relate this object to another object"));
    noSources_->setText(readOnly ? tr("No sources cited.") : tr("No sources cited yet."));
    backAction_->setVisible(!back_.isEmpty());
    backAction_->setEnabled(!saving_);
    if (!back_.isEmpty()) {
        const QString to = back_.last().displayName.isEmpty() ? tr("the previous object") : back_.last().displayName;
        backAction_->setToolTip(tr("Back to %1 (Alt+Left)").arg(to));
    }

    QStringList meta;
    if (!entry_.kind.isEmpty()) { meta << entry_.kind.left(1).toUpper() + entry_.kind.mid(1); }
    if (!entry_.worldID.isEmpty()) {   // I-0142: a label, never a picker
        const QString name = worldName();
        meta << tr("World: %1").arg(name.isEmpty() ? entry_.worldID : name);
    }
    if (!detail_.modifiedAt.isEmpty()) { meta << tr("Modified %1").arg(detail_.modifiedAt); }
    metadata_->setText(meta.join(QLatin1Char('\n')));

    // Chips lose their ✕ when read-only: a ✕ that worked would discard a tag that could never be saved (I-0148).
    if (tagChips_->property("readOnly").toBool() != readOnly) { rebuildTags(); }
}

void ObjectDetailDialog::rebuildTags()
{
    const bool readOnly = isReadOnly();
    tagChips_->setProperty("readOnly", readOnly);
    QLayout* flow = tagChips_->layout();
    while (QLayoutItem* item = flow->takeAt(0)) {
        // ⚠️ deleteLater: a chip's own ✕ is still inside its click handler when the chips are rebuilt.
        if (QWidget* old = item->widget()) {
            old->hide();
            old->setObjectName(QString());
            for (QObject* c : old->findChildren<QObject*>()) { c->setObjectName(QString()); }
            old->deleteLater();
        }
        delete item;
    }
    for (const QString& tag : tags_) {
        auto* chip = new QFrame(tagChips_);
        chip->setObjectName(QStringLiteral("tagChip"));
        chip->setProperty("tag", tag);
        chip->setFrameShape(QFrame::StyledPanel);
        auto* row = new QHBoxLayout(chip);
        row->setContentsMargins(7, 2, readOnly ? 7 : 2, 2);
        row->setSpacing(2);
        row->addWidget(new QLabel(tag, chip));
        if (!readOnly) {
            auto* x = new QToolButton(chip);
            x->setObjectName(QStringLiteral("tagRemove"));
            x->setIcon(GlyphIcon::icon(QStringLiteral("inkwyn.close"), GlyphIcon::Drawing::Small));   // SP-243: the Mac's tag ×
            x->setAutoRaise(true);
            x->setToolTip(tr("Remove the tag “%1”").arg(tag));
            connect(x, &QToolButton::clicked, this, [this, tag] {
                tags_.removeAll(tag);
                rebuildTags();
                refreshState();
            });
            row->addWidget(x);
        }
        flow->addWidget(chip);
    }
    noTags_->setText(readOnly ? tr("No tags.") : tr("No tags yet."));
    noTags_->setVisible(tags_.isEmpty());
    tagChips_->setVisible(!tags_.isEmpty());
}

void ObjectDetailDialog::addTag()
{
    const QString tag = newTag_->text().trimmed();
    newTag_->clear();
    if (tag.isEmpty() || tags_.contains(tag)) { return; }   // two identical chips could not be removed one at a time
    tags_.append(tag);
    rebuildTags();
    refreshState();
}

void ObjectDetailDialog::save()
{
    if (!hasDetail_ || saving_ || isReadOnly() || !hasChanges() || bridge_ == nullptr) { return; }
    saving_ = true;
    refreshState();
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_;
    const Entry entry = entry_;
    const Detail original = detail_;
    const QString name = name_->text(), subtitle = subtitle_->text(), notes = notes_->toPlainText();
    const QStringList tags = tags_;
    struct Saved {
        Write write;
        QJsonObject json;
    };
    AsyncCall::run<Saved>(
        this,
        [handle, root, entry, original, name, subtitle, notes, tags]() -> Saved {
            WorkerBridge bridge(handle);
            Saved s;
            bool away = false;
            QObject::connect(&*bridge, &ScriviBridge::errorOccurred, [&away](int, const QString& message) {
                if (isWorldAwayMessage(message)) { away = true; }
            });
            // I-0155: patch the document as it is on disk NOW, and take from disk every field the writer did not edit.
            const QVariantMap opened = bridge->openObject(root, entry.kind, entry.objectID, entry.worldID);
            const QJsonObject current =
                QJsonDocument::fromJson(opened.value(QStringLiteral("objectJson")).toString().toUtf8()).object();
            if (bridge->lastCallFailed() || current.isEmpty()) {
                s.write.worldAway = away;
                s.write.error = QObject::tr("“%1” could not be read, so nothing was saved.").arg(original.displayName);
                return s;
            }
            const Detail disk = parse(current);
            s.json = patched(current, name == original.displayName ? disk.displayName : name,
                             subtitle == original.subtitle ? disk.subtitle : subtitle,
                             notes == original.notes ? disk.notes : notes, tags == original.tags ? disk.tags : tags);
            bridge->saveObject(root, entry.kind, QString::fromUtf8(QJsonDocument(s.json).toJson(QJsonDocument::Compact)));
            if (bridge->lastCallFailed()) {
                s.write.worldAway = away;
                s.write.error = QObject::tr("“%1” could not be saved.").arg(original.displayName);
            }
            return s;
        },
        [this](const Saved& s) {
            saving_ = false;
            if (s.write.worldAway) {
                // B4: the world went away under the dialog. Keep the drafts; the banners say what is held and why.
                awayLatched_ = true;
                closeAfterSave_ = false;
                afterSave_ = nullptr;
                saveError_->hide();
                rebuildTags();
                refreshState();
                return;
            }
            if (!s.write.error.isEmpty()) {
                closeAfterSave_ = false;   // a failed save never closes: her typing is not thrown away
                afterSave_ = nullptr;      // … and never navigates away from it
                saveError_->setText(s.write.error);
                saveError_->show();
                refreshState();
                return;
            }
            saveError_->hide();
            // What was written is what disk holds: the drafts now match it, so the status can say so at once.
            detail_ = parse(s.json);
            {
                const QSignalBlocker b1(name_), b2(subtitle_), b3(notes_);
                name_->setText(detail_.displayName);
                subtitle_->setText(detail_.subtitle);
                notes_->setPlainText(detail_.notes);
                tags_ = detail_.tags;
            }
            rebuildTags();
            refreshState();
            setStatus(tr("Changes saved"));
            emit saved();
            if (std::exchange(closeAfterSave_, false)) {
                finishClose();
                return;
            }
            if (afterSave_) {
                std::exchange(afterSave_, nullptr)();   // the navigation the prompt asked to save before
                return;
            }
            reload();
        },
        [this] {
            saving_ = false;
            closeAfterSave_ = false;
            afterSave_ = nullptr;
            saveError_->setText(tr("Saving is taking longer than expected — the project may be on a slow or unreachable "
                                   "volume. Your changes are still here."));
            saveError_->show();
            refreshState();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::revert()
{
    if (!hasDetail_ || saving_) { return; }
    {
        const QSignalBlocker b1(name_), b2(subtitle_), b3(notes_);
        name_->setText(detail_.displayName);
        subtitle_->setText(detail_.subtitle);
        notes_->setPlainText(detail_.notes);
        tags_ = detail_.tags;
    }
    saveError_->hide();
    rebuildTags();
    refreshState();
    setStatus(tr("Changes reverted"));
    reload();   // "the saved version" is what is on disk now, not what this dialog last saw (I-0155)
}

void ObjectDetailDialog::requestClose()
{
    if (closing_) { return; }
    if (saving_) {
        closeAfterSave_ = true;   // the write lands, then the dialog closes
        return;
    }
    if (!hasChanges() || isReadOnly()) {   // read-only drafts are covered by the at-risk banner (T-0450)
        finishClose();
        return;
    }
    if (findChild<QMessageBox*>(QStringLiteral("saveChangesPrompt")) != nullptr) { return; }
    auto* box = new QMessageBox(this);
    box->setObjectName(QStringLiteral("saveChangesPrompt"));
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(tr("Save your changes?"));
    box->setText(tr("Save your changes?"));
    box->setInformativeText(tr("You have unsaved changes to “%1”. Closing without saving will discard them.")
                                .arg(detail_.displayName.isEmpty() ? tr("this object") : detail_.displayName));
    QPushButton* saveButton = box->addButton(tr("Save"), QMessageBox::AcceptRole);
    QPushButton* discard = box->addButton(tr("Discard"), QMessageBox::DestructiveRole);
    QPushButton* cancel = box->addButton(tr("Cancel"), QMessageBox::RejectRole);
    box->setDefaultButton(saveButton);
    box->setEscapeButton(cancel);
    connect(box, &QMessageBox::finished, this, [this, box, saveButton, discard] {
        if (box->clickedButton() == saveButton) {
            closeAfterSave_ = true;
            save();
        } else if (box->clickedButton() == discard) {
            finishClose();
        }
    });
    box->open();
}

void ObjectDetailDialog::finishClose()
{
    closing_ = true;
    QDialog::reject();
    deleteLater();
}

void ObjectDetailDialog::reject()
{
    if (closing_) {
        QDialog::reject();
        return;
    }
    requestClose();   // Esc honours the unsaved-changes guard exactly as ✕ does (I-0250)
}

void ObjectDetailDialog::closeEvent(QCloseEvent* event)
{
    if (closing_) {
        event->accept();
        return;
    }
    event->ignore();
    requestClose();
}

// ── The image (B2) ──────────────────────────────────────────────────────────────────────────────────────────

void ObjectDetailDialog::runImageWrite(std::function<QString(ScriviBridge&)> work)
{
    if (bridge_ == nullptr || imageWorking_) { return; }
    imageWorking_ = true;
    imageError_->hide();
    refreshState();
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    AsyncCall::run<Write>(
        this,
        [handle, work]() -> Write {
            WorkerBridge bridge(handle);
            Write w;
            QObject::connect(&*bridge, &ScriviBridge::errorOccurred, [&w](int, const QString& message) {
                if (isWorldAwayMessage(message)) { w.worldAway = true; }
            });
            w.error = work(*bridge);
            return w;
        },
        [this](const Write& w) {
            imageWorking_ = false;
            if (!w.error.isEmpty()) {
                if (w.worldAway) { awayLatched_ = true; }
                imageError_->setText(w.worldAway ? tr("This object's world is unavailable, so its image cannot be changed.")
                                                 : w.error);
                imageError_->show();
                rebuildTags();
                refreshState();
                return;
            }
            emit saved();
            reload();   // re-read the object and its image path
        },
        [this] {
            imageWorking_ = false;
            imageError_->setText(tr("The image is taking longer than expected — the project may be on a slow or unreachable "
                                    "volume. It will appear when it lands."));
            imageError_->show();
            refreshState();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::chooseImageFile()
{
    auto* chooser = new QFileDialog(this, tr("Choose an Image"));
    chooser->setObjectName(QStringLiteral("imageFileDialog"));
    chooser->setAttribute(Qt::WA_DeleteOnClose);
    chooser->setFileMode(QFileDialog::ExistingFile);
    chooser->setNameFilter(tr("Images (*.png *.jpg *.jpeg *.gif *.bmp *.webp *.tif *.tiff *.heic)"));
    connect(chooser, &QFileDialog::fileSelected, this, &ObjectDetailDialog::importImage);
    chooser->open();
}

void ObjectDetailDialog::importImage(const QString& path)
{
    if (path.isEmpty()) { return; }
    const QString root = root_, projectID = projectID_, previous = detail_.imageAssetID, previousPath = imagePath_;
    const Entry entry = entry_;
    runImageWrite([root, projectID, previous, previousPath, entry, path](ScriviBridge& b) -> QString {
        const QString assetID = b.importAsset(root, path, QStringLiteral("image"), QFileInfo(path).fileName(),
                                              entry.worldID, projectID)
                                    .value(QStringLiteral("assetID"))
                                    .toString();
        if (b.lastCallFailed() || assetID.isEmpty()) {
            return QObject::tr("“%1” could not be imported.").arg(QFileInfo(path).fileName());
        }
        const QString error = attachImage(b, root, entry.kind, entry.objectID, entry.worldID, assetID);
        if (!error.isEmpty()) { return error; }
        if (previous != assetID) {   // Replace: the old asset goes once the new one is attached, unless it is shared
            removeAssetIfUnshared(b, root, previous, previousPath, entry.objectID, entry.worldID, projectID);
        }
        return {};
    });
}

void ObjectDetailDialog::chooseExisting()
{
    if (bridge_ == nullptr) { return; }
    struct Listed {
        QString assetID, label, filename;
        QImage thumb;
    };
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_, worldID = entry_.worldID, current = detail_.imageAssetID;
    imageExisting_->setEnabled(false);
    AsyncCall::run<QList<Listed>>(
        this,
        [handle, root, worldID]() -> QList<Listed> {
            WorkerBridge bridge(handle);
            QList<Listed> out;
            for (const QVariant& v :
                 bridge->listAssets(root, QStringLiteral("image"), worldID).value(QStringLiteral("assets")).toList()) {
                const QVariantMap m = v.toMap();
                Listed l;
                l.assetID = m.value(QStringLiteral("assetID")).toString();
                l.filename = m.value(QStringLiteral("filename")).toString();
                const QString title = m.value(QStringLiteral("title")).toString();
                l.label = title.isEmpty() ? l.filename : title;
                QImage image(m.value(QStringLiteral("assetPath")).toString());
                if (!image.isNull()) { l.thumb = image.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation); }
                out.append(l);
            }
            return out;
        },
        [this, current](const QList<Listed>& assets) {
            refreshState();
            auto* picker = new QDialog(this);
            picker->setObjectName(QStringLiteral("existingImagePicker"));
            picker->setAttribute(Qt::WA_DeleteOnClose);
            picker->setWindowTitle(tr("Images in this world"));
            auto* col = new QVBoxLayout(picker);
            col->addWidget(new QLabel(tr("Images in this world"), picker));
            auto* list = new QListWidget(picker);
            list->setObjectName(QStringLiteral("existingImageList"));
            list->setIconSize(QSize(32, 32));
            for (const Listed& a : assets) {
                const bool isCurrent = a.assetID == current;
                auto* item = new QListWidgetItem(QIcon(QPixmap::fromImage(a.thumb)),
                                                 isCurrent ? tr("%1  ✓").arg(a.label) : a.label, list);
                item->setData(Qt::UserRole, a.assetID);
                item->setToolTip(isCurrent ? tr("Already this object's image") : a.filename);
                if (isCurrent) { item->setFlags(item->flags() & ~Qt::ItemIsEnabled); }
            }
            auto* empty = new QLabel(tr("No images have been imported into this world yet."), picker);
            empty->setObjectName(QStringLiteral("existingImageEmpty"));
            empty->setVisible(assets.isEmpty());
            list->setVisible(!assets.isEmpty());
            col->addWidget(empty);
            col->addWidget(list);
            auto pick = [this, picker](QListWidgetItem* item) {
                if (item == nullptr || !(item->flags() & Qt::ItemIsEnabled)) { return; }
                const QString assetID = item->data(Qt::UserRole).toString();
                picker->accept();
                attachExisting(assetID);
            };
            connect(list, &QListWidget::itemClicked, picker, pick);
            connect(list, &QListWidget::itemActivated, picker, pick);
            picker->resize(320, 300);
            picker->open();
        },
        [this] {
            refreshState();
            imageError_->setText(tr("This world's images are taking longer than expected to read."));
            imageError_->show();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::attachExisting(const QString& assetID)
{
    if (assetID.isEmpty() || assetID == detail_.imageAssetID) { return; }
    const QString root = root_;
    const Entry entry = entry_;
    runImageWrite([root, entry, assetID](ScriviBridge& b) {
        return attachImage(b, root, entry.kind, entry.objectID, entry.worldID, assetID);
    });
}

void ObjectDetailDialog::removeImage()
{
    const QString assetID = detail_.imageAssetID;
    if (assetID.isEmpty()) { return; }
    const QString root = root_, projectID = projectID_, assetPath = imagePath_;
    const Entry entry = entry_;
    runImageWrite([root, projectID, entry, assetID, assetPath](ScriviBridge& b) -> QString {
        const QString error = attachImage(b, root, entry.kind, entry.objectID, entry.worldID, QString());   // detach first
        if (!error.isEmpty()) { return error; }
        removeAssetIfUnshared(b, root, assetID, assetPath, entry.objectID, entry.worldID, projectID);
        return {};
    });
}

// ── A9: delete and change of kind, as the Mac's ([SP-239]) ─────────────────────────────────────────────────────

namespace {
QString otherKind(const QString& kind)
{
    if (kind == QLatin1String("item")) { return QStringLiteral("artifact"); }
    if (kind == QLatin1String("artifact")) { return QStringLiteral("item"); }
    return {};
}

QString capitalized(const QString& s) { return s.left(1).toUpper() + s.mid(1); }
}   // namespace

void ObjectDetailDialog::fillMoreMenu()
{
    QMenu* menu = more_->menu();
    menu->clear();
    if (const QString target = otherKind(entry_.kind); !target.isEmpty()) {
        if (hasChanges()) { menu->addSection(tr("Save your changes first")); }
        QAction* change = menu->addAction(tr("Change to %1").arg(capitalized(target)));
        change->setObjectName(QStringLiteral("detailChangeKind"));
        change->setEnabled(!hasChanges());   // an edit is never saved under the old kind
        connect(change, &QAction::triggered, this, [this, target] { changeKind(target); });
        if (hasChanges()) { menu->addSeparator(); }
    }
    QAction* del = menu->addAction(tr("Delete %1…").arg(capitalized(entry_.kind.isEmpty() ? QStringLiteral("object")
                                                                                          : entry_.kind)));
    del->setObjectName(QStringLiteral("detailDelete"));
    connect(del, &QAction::triggered, this, &ObjectDetailDialog::confirmDelete);
}

void ObjectDetailDialog::confirmDelete()
{
    if (!hasDetail_ || saving_ || isReadOnly()) { return; }
    const QString name = detail_.displayName;
    const QString unsaved = hasChanges() ? tr(" Your unsaved changes are discarded.") : QString();
    const QString world = worldName();
    auto* box = new QMessageBox(this);
    box->setObjectName(QStringLiteral("deletePrompt"));
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Warning);
    box->setWindowTitle(tr("Delete “%1”?").arg(name));
    box->setText(tr("Delete “%1”?").arg(name));
    box->setInformativeText(
        entry_.worldID.isEmpty()
            ? tr("“%1” will be removed from this project, with its citations and relationships. This can't be undone.%2")
                  .arg(name, unsaved)
            : tr("“%1” will be removed from the world “%2” and from every project that uses it, with its relationships "
                 "in this project. This can't be undone.%3")
                  .arg(name, world.isEmpty() ? entry_.worldID : world, unsaved));
    QPushButton* del = box->addButton(tr("Delete"), QMessageBox::DestructiveRole);
    QPushButton* cancel = box->addButton(tr("Cancel"), QMessageBox::RejectRole);
    box->setDefaultButton(cancel);
    box->setEscapeButton(cancel);
    connect(box, &QMessageBox::finished, this, [this, box, del] {
        if (box->clickedButton() == del) { deleteCurrent(); }
    });
    box->open();
}

void ObjectDetailDialog::runObjectWrite(std::function<QString(ScriviBridge&)> work, std::function<void()> done)
{
    if (bridge_ == nullptr || saving_) { return; }
    saving_ = true;   // Save, Back and ⋯ grey while it runs
    saveError_->hide();
    refreshState();
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    AsyncCall::run<Write>(
        this,
        [handle, work]() -> Write {
            WorkerBridge bridge(handle);
            Write w;
            QObject::connect(&*bridge, &ScriviBridge::errorOccurred, [&w](int, const QString& m) {
                if (isWorldAwayMessage(m)) { w.worldAway = true; }
            });
            w.error = work(*bridge);
            return w;
        },
        [this, done](const Write& w) {
            saving_ = false;
            if (!w.error.isEmpty()) {
                if (w.worldAway) { awayLatched_ = true; }
                saveError_->setText(w.worldAway ? readOnlyBanner() : w.error);
                saveError_->show();
                refreshState();
                return;
            }
            done();
        },
        [this] {
            saving_ = false;
            saveError_->setText(tr("This is taking longer than expected — the project may be on a slow or unreachable "
                                   "volume."));
            saveError_->show();
            refreshState();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::deleteCurrent()
{
    if (!hasDetail_ || isReadOnly()) { return; }
    const QString root = root_;
    const Entry entry = entry_;
    const QString name = detail_.displayName;
    runObjectWrite(
        [root, entry, name](ScriviBridge& b) -> QString {
            b.deleteObject(root, entry.kind, entry.objectID, entry.worldID);
            return b.lastCallFailed() ? QObject::tr("“%1” could not be deleted.").arg(name) : QString();
        },
        [this, entry] {
            emit saved();   // the Inspector's cards drop it
            // [SP-240] Q3: Back must never reach the deleted object — it can be deeper in the trail (A → B → A).
            QList<Entry> kept;
            for (const Entry& e : std::as_const(back_)) {
                if (e.objectID != entry.objectID && (kept.isEmpty() || kept.last().objectID != e.objectID)) { kept << e; }
            }
            back_ = kept;
            if (back_.isEmpty()) {
                finishClose();
                return;
            }
            showEntry(back_.takeLast());
        });
}

void ObjectDetailDialog::changeKind(const QString& target)
{
    if (!hasDetail_ || isReadOnly() || hasChanges()) { return; }
    const QString root = root_;
    const Entry entry = entry_;
    runObjectWrite(
        [root, entry, target](ScriviBridge& b) -> QString {
            b.promoteObject(root, entry.objectID, target, entry.worldID);
            return b.lastCallFailed() ? QObject::tr("This object could not be changed to %1.").arg(target) : QString();
        },
        [this, entry, target] {
            // [SP-240] Q3: every entry of it in the trail takes the new kind, so Back never opens it under the old one.
            for (Entry& e : back_) {
                if (e.objectID == entry.objectID) { e.kind = target; }
            }
            Entry now = entry_;
            now.kind = target;
            if (hasDetail_ && !detail_.displayName.isEmpty()) { now.displayName = detail_.displayName; }
            emit saved();   // it moves from one card to the other
            showEntry(now);
        });
}

// ── C3: navigation inside the window ───────────────────────────────────────────────────────────────────────────

void ObjectDetailDialog::showEntry(const Entry& entry)
{
    entry_ = entry;
    ++readGeneration_;   // a read of the object we left must not land on this one
    hasDetail_ = false;
    detail_ = Detail();
    {
        const QSignalBlocker b1(name_), b2(subtitle_), b3(notes_);
        name_->clear();
        subtitle_->clear();
        notes_->clear();
    }
    tags_.clear();
    imagePath_.clear();
    image_ = QImage();
    awayLatched_ = false;
    edges_.clear();
    edgesRead_ = false;
    for (QLabel* l : {saveError_, imageError_, sourcesError_, relationsError_}) { l->hide(); }
    setStatus({});
    setWindowTitle(entry_.displayName.isEmpty() ? tr("Object") : entry_.displayName);
    messageTitle_->setText(entry_.displayName);
    messageText_->setText(tr("Loading…"));
    messageHint_->clear();
    pages_->setCurrentIndex(kMessagePage);
    rebuildTags();
    rebuildGraph();
    refreshState();
    reload();
}

void ObjectDetailDialog::navigateTo(const Entry& entry)
{
    if (entry.objectID.isEmpty() || entry.objectID == entry_.objectID || saving_) { return; }   // a re-visit is a no-op
    auto push = [this, entry] {
        Entry here = entry_;
        if (hasDetail_ && !detail_.displayName.isEmpty()) { here.displayName = detail_.displayName; }
        back_.append(here);
        showEntry(entry);
    };
    if (!hasChanges() || isReadOnly()) {
        push();
        return;
    }
    if (findChild<QMessageBox*>(QStringLiteral("saveChangesPrompt")) != nullptr) { return; }
    auto* box = new QMessageBox(this);
    box->setObjectName(QStringLiteral("saveChangesPrompt"));
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(tr("Save your changes?"));
    box->setText(tr("Save your changes?"));
    box->setInformativeText(tr("You have unsaved changes to “%1”. Opening “%2” without saving will discard them.")
                                .arg(detail_.displayName, entry.displayName));
    QPushButton* saveButton = box->addButton(tr("Save"), QMessageBox::AcceptRole);
    QPushButton* discard = box->addButton(tr("Discard"), QMessageBox::DestructiveRole);
    QPushButton* cancel = box->addButton(tr("Cancel"), QMessageBox::RejectRole);
    box->setDefaultButton(saveButton);
    box->setEscapeButton(cancel);
    connect(box, &QMessageBox::finished, this, [this, box, saveButton, discard, push] {
        if (box->clickedButton() == saveButton) {
            afterSave_ = push;
            save();
        } else if (box->clickedButton() == discard) {
            push();
        }
    });
    box->open();
}

void ObjectDetailDialog::goBack()
{
    if (back_.isEmpty() || saving_) { return; }
    auto pop = [this] { showEntry(back_.takeLast()); };
    if (!hasChanges() || isReadOnly()) {
        pop();
        return;
    }
    if (findChild<QMessageBox*>(QStringLiteral("backPrompt")) != nullptr) { return; }
    // ⚠️ Save or Discard, never Cancel: the user's ruling on the Mac, 2026-09-23 — *"This confirmation dialog shouldn't
    // attempt to 'Cancel' the navigation … rather it should require either a save or a revert."*
    auto* box = new QMessageBox(this);
    box->setObjectName(QStringLiteral("backPrompt"));
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(tr("Save your changes?"));
    box->setText(tr("Save your changes to “%1”?").arg(detail_.displayName));
    box->setInformativeText(tr("Going back without saving will discard them."));
    QPushButton* saveButton = box->addButton(tr("Save"), QMessageBox::AcceptRole);
    QPushButton* discard = box->addButton(tr("Discard"), QMessageBox::DestructiveRole);
    box->setDefaultButton(saveButton);
    connect(box, &QMessageBox::finished, this, [this, box, saveButton, discard, pop] {
        if (box->clickedButton() == saveButton) {
            afterSave_ = pop;
            save();
        } else if (box->clickedButton() == discard) {
            pop();
        }
    });
    box->open();
}

// ── C1 / C4: the graph ───────────────────────────────────────────────────────────────────────────────────────

const ObjectDetailDialog::Edge* ObjectDetailDialog::edgeFor(const QString& edgeID) const
{
    for (const Edge& e : edges_) {
        if (e.edgeID == edgeID) { return &e; }
    }
    return nullptr;
}

QString ObjectDetailDialog::edgeName(const Edge& edge) const
{
    const QString name = edge.otherName.trimmed();
    if (!name.isEmpty()) { return name; }
    if (edge.otherKind.isEmpty() && sceneNamer) {
        if (const QString scene = sceneNamer(edge.otherID); !scene.isEmpty()) { return scene; }
    }
    return edge.otherKind.isEmpty() ? tr("Untitled scene") : tr("Untitled %1").arg(edge.otherKind);
}

QString ObjectDetailDialog::pendingSentence(const Edge& edge) const
{
    const auto it = worlds_.constFind(edge.otherWorldID);
    const QString subject = (it == worlds_.constEnd() || it->name.isEmpty()) ? tr("its world") : tr("“%1”").arg(it->name);
    return tr("Held pending — %1 is %2.").arg(subject, WorldStatusText::writerDescription(edge.otherWorldStatus));
}

void ObjectDetailDialog::rebuildGraph()
{
    const bool failed = hasDetail_ && !edgesRead_;

    // Sources: the `cites` edges whose other end is a source (Apple: `ObjectSourcesSection.load`).
    sources_->clear();
    for (const Edge& e : edges_) {
        if (e.relationType != kCites || e.otherKind != kSourceKind || e.otherID == entry_.objectID) { continue; }
        auto* item = new QListWidgetItem(edgeName(e), sources_);
        item->setData(Qt::UserRole, e.edgeID);
        item->setToolTip(tr("Click to show the citation; double-click to show the source"));
    }
    // A read failure is said here; a write's own error (runGraphWrite) is left standing until the next write.
    auto readFailure = [failed](QLabel* label, const QString& text) {
        if (failed) {
            label->setText(text);
            label->show();
        } else if (label->text() == text) {
            label->hide();
        }
    };
    readFailure(sourcesError_, tr("This object's sources could not be read."));
    noSources_->setVisible(hasDetail_ && !failed && sources_->count() == 0);
    sources_->setVisible(sources_->count() > 0);
    if (sources_->count() > 0) { fitToRows(sources_, sources_->count()); }

    // Related: grouped by the label the CORE gives from this end — never recomputed (C1). Groups and rows by name.
    relations_->clear();
    QMap<QString, QList<const Edge*>> groups;
    for (const Edge& e : edges_) { groups[e.label].append(&e); }
    QStringList labels = groups.keys();
    std::sort(labels.begin(), labels.end(), [](const QString& a, const QString& b) { return a.localeAwareCompare(b) < 0; });
    int rows = 0;
    for (const QString& label : labels) {
        auto* group = new QTreeWidgetItem(relations_, {label});
        QFont f = group->font(0);
        f.setBold(true);
        group->setFont(0, f);
        group->setFlags(Qt::ItemIsEnabled);
        group->setForeground(0, palette().brush(QPalette::PlaceholderText));
        QList<const Edge*> list = groups.value(label);
        std::sort(list.begin(), list.end(),
                  [this](const Edge* a, const Edge* b) { return edgeName(*a).localeAwareCompare(edgeName(*b)) < 0; });
        ++rows;
        for (const Edge* e : list) {
            // A pending far end stays listed, named and explained (Doc 3: absence is never deletion).
            auto* item = new QTreeWidgetItem(group, {edgeName(*e),
                                                     e->pending ? pendingSentence(*e) : capitalised(e->otherKind)});
            if (e->pending) { item->setIcon(0, GlyphIcon::icon(QStringLiteral("inkwyn.warnings"))); }   // SP-243 Q6: was "⚠ "
            item->setData(0, Qt::UserRole, e->edgeID);
            item->setForeground(1, palette().brush(QPalette::PlaceholderText));
            item->setToolTip(0, e->pending ? pendingSentence(*e) : tr("Double-click to show %1").arg(edgeName(*e)));
            if (e->pending) { item->setForeground(0, palette().brush(QPalette::PlaceholderText)); }
            ++rows;
        }
    }
    relations_->expandAll();
    readFailure(relationsError_, tr("This object's relationships could not be read."));
    noRelations_->setVisible(hasDetail_ && !failed && edges_.isEmpty());
    relations_->setVisible(rows > 0);
    if (rows > 0) { fitToRows(relations_, rows); }
}

void ObjectDetailDialog::activateEdge(const Edge& edge)
{
    if (edge.pending) { return; }   // its world is away: there is nothing to open
    if (edge.otherKind.isEmpty()) {
        emit showSceneRequested(edge.otherID);   // a scene has no detail view (I-0151): the manuscript goes there
        return;
    }
    Entry next;
    next.kind = edge.otherKind;
    next.objectID = edge.otherID;
    next.worldID = edge.otherKind == kSourceKind ? QString() : edge.otherWorldID;
    next.displayName = edgeName(edge);
    navigateTo(next);
}

void ObjectDetailDialog::relationMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = relations_->itemAt(pos);
    const Edge* edge = item != nullptr ? edgeFor(item->data(0, Qt::UserRole).toString()) : nullptr;
    if (edge == nullptr) { return; }
    relations_->setCurrentItem(item);   // the row a right-click targets is the row selected (I-0158)
    const Edge e = *edge;
    auto* menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("relationMenu"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(tr("Show"), this, [this, e] { activateEdge(e); })->setEnabled(!e.pending);
    QAction* remove = menu->addAction(tr("Remove Relationship"), this, [this, e] {
        const QString root = root_, edgeID = e.edgeID;
        runGraphWrite(relationsError_, [root, edgeID](ScriviBridge& b) {
            b.deleteEdge(root, edgeID);
            return b.lastCallFailed() ? QObject::tr("The relationship could not be removed.") : QString();
        });
    });
    remove->setEnabled(!e.pending && !isReadOnly() && !graphWorking_);
    menu->popup(relations_->viewport()->mapToGlobal(pos));
}

void ObjectDetailDialog::sourceMenu(const QPoint& pos)
{
    QListWidgetItem* item = sources_->itemAt(pos);
    const Edge* edge = item != nullptr ? edgeFor(item->data(Qt::UserRole).toString()) : nullptr;
    if (edge == nullptr) { return; }
    sources_->setCurrentItem(item);
    const Edge e = *edge;
    auto* menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("sourceMenu"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(tr("Show Citation"), this, [this, e] { citeOf(e); });
    menu->addAction(tr("Show Source"), this, [this, e] { activateEdge(e); });
    menu->addSeparator();
    QAction* remove = menu->addAction(tr("Remove Citation"), this, [this, e] {
        const QString root = root_, edgeID = e.edgeID;
        runGraphWrite(sourcesError_, [root, edgeID](ScriviBridge& b) {
            b.deleteEdge(root, edgeID);
            return b.lastCallFailed() ? QObject::tr("The citation could not be removed.") : QString();
        });
    });
    remove->setEnabled(!isReadOnly() && !graphWorking_);
    menu->popup(sources_->viewport()->mapToGlobal(pos));
}

void ObjectDetailDialog::runGraphWrite(QLabel* errorLabel, std::function<QString(ScriviBridge&)> work)
{
    if (bridge_ == nullptr || graphWorking_) { return; }
    graphWorking_ = true;
    errorLabel->hide();
    refreshState();
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    AsyncCall::run<Write>(
        this,
        [handle, work]() -> Write {
            WorkerBridge bridge(handle);
            Write w;
            QObject::connect(&*bridge, &ScriviBridge::errorOccurred, [&w](int, const QString& message) {
                if (isWorldAwayMessage(message)) { w.worldAway = true; }
            });
            w.error = work(*bridge);
            return w;
        },
        [this, errorLabel](const Write& w) {
            graphWorking_ = false;
            if (!w.error.isEmpty()) {
                if (w.worldAway) { awayLatched_ = true; }
                errorLabel->setText(w.worldAway ? tr("This object's world is unavailable, so its relationships are frozen.")
                                                : w.error);
                errorLabel->show();
            }
            emit saved();   // the graph may have changed even on a partial failure: the Inspector reads the same edges
            reload();
        },
        [this, errorLabel] {
            graphWorking_ = false;
            errorLabel->setText(tr("This is taking longer than expected — the project may be on a slow or unreachable "
                                   "volume. It will appear when it lands."));
            errorLabel->show();
            refreshState();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::chooseRelation(const QString& selectCode)
{
    if (bridge_ == nullptr) { return; }
    struct Obj {
        QString id, name, kind, worldID;
    };
    struct Type {
        QString code, label, sourceKind, targetKind;
    };
    struct Listed {
        QList<Obj> objects;
        QList<Type> types;
        QMap<QString, QString> worldNames;
        bool failed = false;
    };
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_;
    relate_->setEnabled(false);
    AsyncCall::run<Listed>(
        this,
        [handle, root]() -> Listed {
            WorkerBridge bridge(handle);
            Listed l;
            for (const QVariant& v : bridge->listObjects(root, QString()).value(QStringLiteral("objects")).toList()) {
                const QVariantMap m = v.toMap();
                l.objects.append({m.value(QStringLiteral("objectID")).toString(), m.value(QStringLiteral("displayName")).toString(),
                                  m.value(QStringLiteral("kind")).toString(), m.value(QStringLiteral("worldID")).toString()});
            }
            l.failed = bridge->lastCallFailed();
            for (const QVariant& v : bridge->listWorlds(root).value(QStringLiteral("worlds")).toList()) {
                const QVariantMap m = v.toMap();
                l.worldNames.insert(m.value(QStringLiteral("worldID")).toString(), m.value(QStringLiteral("displayName")).toString());
            }
            for (const QVariant& v : bridge->listRelationTypes(root).value(QStringLiteral("types")).toList()) {
                const QVariantMap m = v.toMap();
                l.types.append({m.value(QStringLiteral("code")).toString(), m.value(QStringLiteral("forwardLabel")).toString(),
                                m.value(QStringLiteral("sourceKind")).toString(), m.value(QStringLiteral("targetKind")).toString()});
            }
            l.failed = l.failed || bridge->lastCallFailed();
            return l;
        },
        [this, selectCode](const Listed& listed) {
            refreshState();
            const QString scene = QStringLiteral("scene");
            QList<Type> usable;   // Apple: `ObjectRelationPicker.usableTypes` — from this kind, to an object
            for (const Type& t : listed.types) {
                if ((t.sourceKind.isEmpty() || t.sourceKind == entry_.kind) && t.sourceKind != scene && t.targetKind != scene) {
                    usable.append(t);
                }
            }
            QSet<QString> related;
            for (const Edge& e : edges_) { related.insert(e.otherID); }

            auto* picker = new QDialog(this);
            picker->setObjectName(QStringLiteral("relatePicker"));
            picker->setAttribute(Qt::WA_DeleteOnClose);
            picker->setWindowTitle(tr("Relate to…"));
            auto* col = new QVBoxLayout(picker);
            auto* title = new QLabel(tr("Relate to…"), picker);
            QFont f = title->font();
            f.setBold(true);
            title->setFont(f);
            col->addWidget(title);
            if (listed.failed) {
                QLabel* why = warning("relateError", picker);
                why->setText(tr("This project's objects could not all be read."));
                why->show();
                col->addWidget(why);
            }
            QComboBox* typeBox = nullptr;
            auto* typeRow = new QHBoxLayout;
            if (usable.size() > 1) {
                typeBox = new QComboBox(picker);
                typeBox->setObjectName(QStringLiteral("relateType"));
                for (const Type& t : usable) { typeBox->addItem(t.label, t.code); }
                typeBox->setCurrentIndex(std::max(0, typeBox->findData(selectCode)));
                typeRow->addWidget(typeBox, 1);
            } else if (usable.size() == 1) {
                auto* only = new QLabel(usable.first().label, picker);
                only->setObjectName(QStringLiteral("relateTypeLabel"));
                typeRow->addWidget(only, 1);
            } else {
                typeRow->addStretch(1);
            }
            // [SP-238] (EP-059 C5): the Mac's New Type… — the form over the picker, From set to this object's kind; the
            // picker comes back with the new type chosen.
            auto* newType = new QPushButton(tr("New Type…"), picker);
            newType->setObjectName(QStringLiteral("relateNewType"));
            typeRow->addWidget(newType);
            col->addLayout(typeRow);
            connect(newType, &QPushButton::clicked, picker, [this, picker] {
                auto* form = new RelationTypeDialog(bridge_, root_, {}, entry_.kind, picker);
                connect(form, &QDialog::accepted, this, [this, picker, form] {
                    const QString code = form->savedCode();
                    picker->close();
                    chooseRelation(code);
                });
                form->open();
            });
            auto* search = new QLineEdit(picker);
            search->setObjectName(QStringLiteral("relateSearch"));
            search->setPlaceholderText(tr("Search all worlds…"));
            col->addWidget(search);
            auto* hint = new QLabel(tr("Objects from every world in this project are listed."), picker);
            hint->setForegroundRole(QPalette::PlaceholderText);
            col->addWidget(hint);
            auto* empty = new QLabel(picker);
            empty->setObjectName(QStringLiteral("relateEmpty"));
            empty->setWordWrap(true);
            empty->setForegroundRole(QPalette::PlaceholderText);
            col->addWidget(empty);
            auto* list = new QListWidget(picker);
            list->setObjectName(QStringLiteral("relateList"));
            col->addWidget(list, 1);

            const QList<Obj> objects = listed.objects;
            const QMap<QString, QString> worldNames = listed.worldNames;
            const QString self = entry_.objectID;
            auto selectedType = [typeBox, usable]() -> Type {
                if (usable.isEmpty()) { return {}; }
                return typeBox != nullptr ? usable.value(typeBox->currentIndex()) : usable.first();
            };
            auto fill = [list, empty, search, objects, worldNames, self, related, usable, selectedType] {
                list->clear();
                const QString q = search->text().trimmed();
                QMap<QString, QList<Obj>> byWorld;   // "" = project-scoped, which sorts first
                for (const Obj& o : objects) {
                    if (o.id == self || (!q.isEmpty() && !o.name.contains(q, Qt::CaseInsensitive))) { continue; }
                    byWorld[o.worldID].append(o);
                }
                const Type type = selectedType();
                for (auto it = byWorld.begin(); it != byWorld.end(); ++it) {
                    if (!it.key().isEmpty()) {
                        auto* header = new QListWidgetItem(worldNames.value(it.key(), it.key()), list);
                        header->setFlags(Qt::NoItemFlags);
                        QFont hf = header->font();
                        hf.setBold(true);
                        header->setFont(hf);
                    }
                    QList<Obj> group = it.value();
                    std::sort(group.begin(), group.end(),
                              [](const Obj& a, const Obj& b) { return a.name.localeAwareCompare(b.name) < 0; });
                    for (const Obj& o : group) {
                        const bool already = related.contains(o.id);
                        const bool accepted = type.targetKind.isEmpty() || type.targetKind == o.kind;
                        auto* item = new QListWidgetItem(
                            already ? tr("%1  ·  %2  ✓").arg(o.name, capitalised(o.kind)) : tr("%1  ·  %2").arg(o.name, capitalised(o.kind)),
                            list);
                        item->setData(Qt::UserRole, o.id);
                        item->setToolTip(already ? tr("Already related.")
                                                 : accepted ? tr("Relate to %1").arg(o.name)
                                                            : tr("“%1” cannot point at a %2.").arg(type.label, o.kind));
                        if (already || !accepted) { item->setFlags(item->flags() & ~Qt::ItemIsEnabled); }
                    }
                }
                const bool none = list->count() == 0;
                empty->setText(usable.isEmpty() ? tr("This project has no relation type that can link two objects.")
                               : objects.size() <= 1 ? tr("There are no other objects in this project yet.")
                                                     : tr("No match."));
                empty->setVisible(usable.isEmpty() || none);
                list->setVisible(!usable.isEmpty() && !none);
            };
            fill();
            connect(search, &QLineEdit::textChanged, picker, fill);
            if (typeBox != nullptr) { connect(typeBox, &QComboBox::currentIndexChanged, picker, fill); }
            auto pick = [this, picker, selectedType](QListWidgetItem* item) {
                if (item == nullptr || !(item->flags() & Qt::ItemIsEnabled)) { return; }
                const QString target = item->data(Qt::UserRole).toString(), code = selectedType().code;
                if (target.isEmpty() || code.isEmpty()) { return; }
                picker->accept();
                const QString root = root_, from = entry_.objectID;
                runGraphWrite(relationsError_, [root, from, target, code](ScriviBridge& b) {
                    b.createEdge(root, from, target, code, QString());
                    return b.lastCallFailed() ? QObject::tr("The relationship could not be created.") : QString();
                });
            };
            connect(list, &QListWidget::itemClicked, picker, pick);
            connect(list, &QListWidget::itemActivated, picker, pick);
            picker->resize(360, 420);
            picker->open();
        },
        [this] {
            refreshState();
            relationsError_->setText(tr("This project's objects are taking longer than expected to read."));
            relationsError_->show();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::chooseSource()
{
    if (bridge_ == nullptr) { return; }
    struct Listed {
        QList<QPair<QString, QString>> sources;   // id, name
        bool failed = false;
    };
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_;
    cite_->setEnabled(false);
    AsyncCall::run<Listed>(
        this,
        [handle, root]() -> Listed {
            WorkerBridge bridge(handle);
            Listed l;
            for (const QVariant& v : bridge->listObjects(root, kSourceKind).value(QStringLiteral("objects")).toList()) {
                const QVariantMap m = v.toMap();
                l.sources.append({m.value(QStringLiteral("objectID")).toString(), m.value(QStringLiteral("displayName")).toString()});
            }
            l.failed = bridge->lastCallFailed();
            return l;
        },
        [this](const Listed& listed) {
            refreshState();
            QSet<QString> cited;
            for (const Edge& e : edges_) {
                if (e.relationType == kCites) { cited.insert(e.otherID); }
            }
            auto* picker = new QDialog(this);
            picker->setObjectName(QStringLiteral("citePicker"));
            picker->setAttribute(Qt::WA_DeleteOnClose);
            picker->setWindowTitle(tr("Cite an existing source"));
            auto* col = new QVBoxLayout(picker);
            auto* title = new QLabel(tr("Cite an existing source"), picker);
            QFont f = title->font();
            f.setBold(true);
            title->setFont(f);
            col->addWidget(title);
            if (listed.failed) {
                QLabel* why = warning("citeError", picker);
                why->setText(tr("This project's sources could not be read."));
                why->show();
                col->addWidget(why);
            }
            auto* search = new QLineEdit(picker);
            search->setObjectName(QStringLiteral("citeSearch"));
            search->setPlaceholderText(tr("Search sources…"));
            col->addWidget(search);
            auto* empty = new QLabel(picker);
            empty->setObjectName(QStringLiteral("citeEmpty"));
            empty->setWordWrap(true);
            empty->setForegroundRole(QPalette::PlaceholderText);
            col->addWidget(empty);
            auto* list = new QListWidget(picker);
            list->setObjectName(QStringLiteral("citeList"));
            col->addWidget(list, 1);
            const auto sources = listed.sources;
            auto fill = [list, empty, search, sources, cited] {
                list->clear();
                const QString q = search->text().trimmed();
                for (const auto& [id, name] : sources) {
                    if (!q.isEmpty() && !name.contains(q, Qt::CaseInsensitive)) { continue; }
                    const bool already = cited.contains(id);
                    auto* item = new QListWidgetItem(already ? tr("%1 — Already cited here").arg(name) : name, list);
                    item->setData(Qt::UserRole, id);
                    item->setToolTip(already ? tr("“%1” is already cited by this object.").arg(name) : tr("Cite “%1”").arg(name));
                    if (already) { item->setFlags(item->flags() & ~Qt::ItemIsEnabled); }
                }
                empty->setText(sources.isEmpty() ? tr("This project has no sources yet. Use “New Source…” to create one.")
                                                 : tr("No match."));
                empty->setVisible(list->count() == 0);
                list->setVisible(list->count() > 0);
            };
            fill();
            connect(search, &QLineEdit::textChanged, picker, fill);
            auto pick = [this, picker](QListWidgetItem* item) {
                if (item == nullptr || !(item->flags() & Qt::ItemIsEnabled)) { return; }
                const QString sourceID = item->data(Qt::UserRole).toString();
                picker->accept();
                const QString root = root_, objectID = entry_.objectID;
                runGraphWrite(sourcesError_, [root, sourceID, objectID](ScriviBridge& b) {
                    b.createEdge(root, sourceID, objectID, kCites, QString());   // source —cites→ object
                    return b.lastCallFailed() ? QObject::tr("The source could not be cited.") : QString();
                });
            };
            connect(list, &QListWidget::itemClicked, picker, pick);
            connect(list, &QListWidget::itemActivated, picker, pick);
            picker->resize(340, 380);
            picker->open();
        },
        [this] {
            refreshState();
            sourcesError_->setText(tr("This project's sources are taking longer than expected to read."));
            sourcesError_->show();
        },
        AsyncCall::kProjectOpenTimeoutMs);
}

void ObjectDetailDialog::newSource()
{
    const QString documenting = detail_.displayName;
    auto* form = new QDialog(this);
    form->setObjectName(QStringLiteral("newSourceDialog"));
    form->setAttribute(Qt::WA_DeleteOnClose);
    form->setWindowTitle(tr("New source"));
    auto* col = new QVBoxLayout(form);
    auto* title = new QLabel(tr("New source"), form);
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);
    col->addWidget(title);
    auto* what = new QLabel(tr("Documenting “%1”.").arg(documenting), form);
    what->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(what);
    auto* field = new QLineEdit(form);
    field->setObjectName(QStringLiteral("newSourceTitle"));
    field->setPlaceholderText(tr("Title of the work"));
    field->setMinimumWidth(260);
    col->addWidget(field);
    // ⚠️ Not the Mac's sentence: it says the rest is "edited on the source itself", and nothing edits it yet ([SP-236] Q4).
    auto* hint = new QLabel(tr("Author, year and the rest are shown in the citation once they are set."), form);
    hint->setWordWrap(true);
    hint->setForegroundRole(QPalette::PlaceholderText);
    col->addWidget(hint);
    auto* buttons = new QDialogButtonBox(form);
    QPushButton* create = buttons->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    create->setObjectName(QStringLiteral("newSourceCreate"));
    buttons->addButton(QDialogButtonBox::Cancel);
    create->setDefault(true);
    create->setEnabled(false);
    col->addWidget(buttons);
    connect(field, &QLineEdit::textChanged, form, [create](const QString& t) { create->setEnabled(!t.trimmed().isEmpty()); });
    connect(buttons, &QDialogButtonBox::rejected, form, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, form, [this, form, field, documenting] {
        const QString name = field->text().trimmed();
        if (name.isEmpty()) { return; }
        form->accept();
        const QString root = root_, objectID = entry_.objectID;
        runGraphWrite(sourcesError_, [root, objectID, name, documenting](ScriviBridge& b) -> QString {
            // `source` is the one project-scoped kind: no worldID (SP-104).
            const QString sourceID = b.createObject(root, kSourceKind, name, QString(), QString())
                                         .value(QStringLiteral("objectID")).toString();
            if (b.lastCallFailed() || sourceID.isEmpty()) { return QObject::tr("The source could not be created."); }
            b.createEdge(root, sourceID, objectID, kCites, QString());
            if (b.lastCallFailed()) {   // the source is kept: it is the writer's work, and Cite… can attach it
                return QObject::tr("“%1” was created, but could not be attached to “%2”. The source is saved — use “Cite…” "
                                   "to attach it.").arg(name, documenting);
            }
            return {};
        });
    });
    form->open();
}

void ObjectDetailDialog::citeOf(const Edge& edge)
{
    showCitation(this, bridge_, root_, edge.otherID, edgeName(edge), entry_.objectID, detail_.displayName, QString());
}

void ObjectDetailDialog::showCitation(QWidget* parent, ScriviBridge* bridge, const QString& root, const QString& sourceID,
                                      const QString& displayName, const QString& citingObjectID, const QString& citingName,
                                      const QString& attribution)
{
    if (bridge == nullptr || parent == nullptr) { return; }
    struct Record {
        QString name, subtitle, notes, attribution;
        QList<QPair<QString, QString>> fields;
    };
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge);
    AsyncCall::run<Record>(
        parent,
        [handle, root, sourceID, displayName, citingObjectID, citingName, attribution]() -> Record {
            WorkerBridge b(handle);
            Record r;
            r.name = displayName;
            r.attribution = attribution;
            const QJsonObject json = QJsonDocument::fromJson(b->openObject(root, kSourceKind, sourceID, QString())
                                                                 .value(QStringLiteral("objectJson")).toString().toUtf8())
                                         .object();
            if (!json.isEmpty()) {
                r.name = json.value(QStringLiteral("displayName")).toString(displayName);
                r.subtitle = json.value(QStringLiteral("subtitle")).toString();
                r.notes = json.value(QStringLiteral("notes")).toString();
                // T-0453: `attributes` is an ARRAY of {"k","v"}, not an object.
                QMap<QString, QString> attrs;
                for (const QJsonValue& v : json.value(QStringLiteral("attributes")).toArray()) {
                    attrs.insert(v.toObject().value(QStringLiteral("k")).toString(), v.toObject().value(QStringLiteral("v")).toString());
                }
                // Apple's `CitationField`, in its order and words.
                const QList<QPair<QString, QString>> known{{QStringLiteral("author"), QObject::tr("Author")},
                                                           {QStringLiteral("year"), QObject::tr("Year")},
                                                           {QStringLiteral("publisher"), QObject::tr("Publisher")},
                                                           {QStringLiteral("page"), QObject::tr("Page")},
                                                           {QStringLiteral("url"), QObject::tr("URL")},
                                                           {QStringLiteral("accessed"), QObject::tr("Accessed")}};
                for (const auto& [key, label] : known) {
                    const QString value = attrs.value(key).trimmed();
                    if (!value.isEmpty()) { r.fields.append({label, value}); }
                }
            }
            if (!citingObjectID.isEmpty()) {
                QStringList others;
                for (const QVariant& v : b->listEdgesFor(root, sourceID).value(QStringLiteral("edges")).toList()) {
                    const QVariantMap m = v.toMap();
                    const QString other = m.value(QStringLiteral("otherID")).toString();
                    const QString name = m.value(QStringLiteral("otherDisplayName")).toString();
                    if (m.value(QStringLiteral("relationType")).toString() == kCites && other != sourceID && other != citingObjectID
                        && !name.isEmpty() && !others.contains(name)) {
                        others << name;
                    }
                }
                r.attribution = others.isEmpty() ? QObject::tr("Cited by %1").arg(citingName)
                                                 : QObject::tr("Cited by %1, and by %2").arg(citingName, others.join(QStringLiteral(", ")));
            }
            return r;
        },
        [parent](const Record& r) {
            auto* popup = new QDialog(parent);
            popup->setObjectName(QStringLiteral("citationPopup"));
            popup->setAttribute(Qt::WA_DeleteOnClose);
            popup->setWindowTitle(r.name);
            auto* col = new QVBoxLayout(popup);
            auto* name = new QLabel(r.name, popup);
            name->setObjectName(QStringLiteral("citationName"));
            QFont f = name->font();
            f.setBold(true);
            name->setFont(f);
            col->addWidget(name);
            if (!r.subtitle.isEmpty()) { col->addWidget(new QLabel(r.subtitle, popup)); }
            auto* by = new QLabel(r.attribution, popup);
            by->setObjectName(QStringLiteral("citationAttribution"));
            by->setWordWrap(true);
            by->setForegroundRole(QPalette::PlaceholderText);
            col->addWidget(by);
            if (!r.fields.isEmpty()) {
                auto* fields = new QWidget(popup);
                fields->setObjectName(QStringLiteral("citationFields"));
                auto* form = new QFormLayout(fields);
                form->setContentsMargins(0, 4, 0, 4);
                for (const auto& [label, value] : r.fields) {
                    auto* v = new QLabel(value, fields);
                    v->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    v->setWordWrap(true);
                    form->addRow(label, v);
                }
                col->addWidget(fields);
            }
            if (!r.notes.isEmpty()) {
                auto* notes = new QLabel(r.notes, popup);
                notes->setWordWrap(true);
                col->addWidget(notes);
            }
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, popup);
            QObject::connect(buttons, &QDialogButtonBox::rejected, popup, &QDialog::reject);
            col->addWidget(buttons);
            popup->setMinimumWidth(300);
            popup->open();
        },
        [] {},
        AsyncCall::kProjectOpenTimeoutMs);
}
