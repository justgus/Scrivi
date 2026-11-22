#include "ObjectDetailDialog.hpp"

#include "AsyncCall.hpp"
#include "ScriviBridge.hpp"
#include "WorldStatusText.hpp"

#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeySequence>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace {

constexpr int kContentPage = 0;
constexpr int kMessagePage = 1;
const QSize kPreviewBound(480, 220);   // Apple: scaled to fit, at most 220 high

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
    closeAction_ = bar->addAction(QIcon::fromTheme(QStringLiteral("window-close"),
                                                   style()->standardIcon(QStyle::SP_DialogCloseButton)),
                                  tr("Close"));
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

    auto* rule2 = new QFrame(content);
    rule2->setFrameShape(QFrame::HLine);
    rule2->setFrameShadow(QFrame::Sunken);
    col->addWidget(rule2);
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
            x->setText(QStringLiteral("✕"));
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
                saveError_->hide();
                rebuildTags();
                refreshState();
                return;
            }
            if (!s.write.error.isEmpty()) {
                closeAfterSave_ = false;   // a failed save never closes: her typing is not thrown away
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
            reload();
        },
        [this] {
            saving_ = false;
            closeAfterSave_ = false;
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
