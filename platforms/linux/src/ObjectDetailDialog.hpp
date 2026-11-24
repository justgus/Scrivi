#pragma once

// [SP-235] EP-059 B1–B4 — the object detail view, as Apple's `ObjectDetailSheet` + `ObjectImageSection`.
//
// A window-modal dialog over the project window (Q1): Name, Subtitle, Notes, Tags, the image and the metadata. Save patches the
// object as it is on disk now, merged per field (I-0155); a re-read keeps what the writer edited (I-0165b); an object whose world
// is away is shown read-only and explained, never written (R9); a world going away while open loses nothing (I-0165, T-0450).
// ⚠️ It reads nothing from its host (Apple's S8): everything arrives as a parameter, and every read and write runs on a worker.
//
// [SP-236] EP-059 C1–C4 — Sources and Related below the image (`ObjectSourcesSection`, `ObjectRelationsSection`), and
// navigation inside the window: a related object or source opens in place, Back returns (Apple's `NavigationStack`; no
// Forward, user ruling 2026-09-23).

#include <QDialog>
#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QStringList>

#include <functional>

class QAction;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QPushButton;
class QToolButton;
class QStackedWidget;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;
class ScriviBridge;

class ObjectDetailDialog : public QDialog
{
    Q_OBJECT
public:
    struct Entry {
        QString kind;
        QString objectID;
        QString worldID;       // empty for a project-scoped object; threaded through every call
        QString displayName;   // from the row, so a world away at open is still NAMED (I-0166)
        bool pending = false;  // the row already knows its world is away: do not call into a dead volume
    };

    ObjectDetailDialog(ScriviBridge* bridge, const QString& projectRoot, const QString& projectID, const Entry& entry,
                       QWidget* parent = nullptr);

    // Re-read from disk (and the worlds). Fields the writer has edited are kept (I-0165b).
    // The host calls it when the worlds change (Apple: `worldRevision`, I-0162).
    void reload();

    [[nodiscard]] bool hasChanges() const;
    [[nodiscard]] bool isReadOnly() const;

    // C3: open another object in this window. Unsaved changes ask Save · Discard · Cancel first (the close prompt).
    void navigateTo(const Entry& entry);

    // A scene the core has no title for is named as the Navigator names it (Apple's `sceneNames`, SP-118).
    std::function<QString(const QString& sceneID)> sceneNamer;

    // C4: a source's citation record — the fields that are set, who cites it — in a small window over `parent`.
    // `citingName` non-empty: "Cited by <citingName>, and by <others>" is read; empty: `attribution` is shown as given.
    static void showCitation(QWidget* parent, ScriviBridge* bridge, const QString& root, const QString& sourceID,
                             const QString& displayName, const QString& citingObjectID, const QString& citingName,
                             const QString& attribution);

public slots:
    void save();
    void revert();
    void requestClose();   // every exit: ✕, Esc, the title bar — asks when there are unsaved changes (T-0452)
    void goBack();         // C3: with unsaved changes asks Save · Discard — no Cancel (user ruling, 2026-09-23)

signals:
    void saved();   // a write landed: the host refreshes the Inspector (I-0155)
    void showSceneRequested(const QString& sceneID);   // a related SCENE: it has no detail view; the manuscript goes there

protected:
    void reject() override;
    void closeEvent(QCloseEvent* event) override;

private:
    struct Detail {
        QJsonObject json;
        QString displayName, subtitle, notes, modifiedAt, imageAssetID;
        QStringList tags;
        QMap<QString, QString> attributes;   // [I-0370]: a source's citation fields (`attributes` [{"k","v"}])
    };
    struct World {
        QString name, status, reason;
    };
    struct Edge {
        QString edgeID, relationType, label, otherID, otherName, otherKind, otherWorldID, otherWorldStatus;
        bool pending = false;
    };
    struct Read {
        bool opened = false;
        bool worldAway = false;
        bool worldsRead = false;
        QString error;
        QJsonObject json;
        QString imagePath;
        QImage image;
        QMap<QString, World> worlds;
        QList<Edge> edges;
        bool edgesRead = false;
    };
    struct Write {
        QString error;
        bool worldAway = false;
    };

    static Detail parse(const QJsonObject& json);
    void applyRead(const Read& read);
    void refreshState();
    void rebuildTags();
    void addTag();
    void setStatus(const QString& text);
    QString worldName() const;
    QString readOnlyBanner() const;

    // An image change, on a worker: `work` returns an error or empty.
    void runImageWrite(std::function<QString(ScriviBridge&)> work);
    void chooseImageFile();
    void importImage(const QString& path);
    void chooseExisting();
    void attachExisting(const QString& assetID);
    void removeImage();
    void finishClose();

    // A8/A9 ([SP-240], the Mac's `moreMenu`): ⋯ ▸ Change to Artifact / Item, Delete <Kind>….
    void fillMoreMenu();
    void confirmDelete();
    void deleteCurrent();
    void changeKind(const QString& target);
    // `work` returns an error or empty; `done` runs on success, on the main thread.
    void runObjectWrite(std::function<QString(ScriviBridge&)> work, std::function<void()> done);

    // C1–C4.
    void showEntry(const Entry& entry);   // the trail moved: start over on `entry`
    void rebuildGraph();
    QString edgeName(const Edge& edge) const;
    QString pendingSentence(const Edge& edge) const;
    void activateEdge(const Edge& edge);
    void relationMenu(const QPoint& pos);
    void sourceMenu(const QPoint& pos);
    void runGraphWrite(QLabel* errorLabel, std::function<QString(ScriviBridge&)> work);
    // `selectCode`: the type to choose when it fits (New Type… reopens the picker with the type it made).
    void chooseRelation(const QString& selectCode = QString());
    void chooseSource();
    void newSource();
    void citeOf(const Edge& edge);
    [[nodiscard]] const Edge* edgeFor(const QString& edgeID) const;

    ScriviBridge* bridge_;
    QString root_;
    QString projectID_;
    Entry entry_;

    bool hasDetail_ = false;
    Detail detail_;
    QStringList tags_;
    // [I-0370]: a source's editable citation fields, by key (Apple's `CitationField` minus Page, SP-244 Q5), and Page shown.
    QWidget* citationBox_ = nullptr;
    QMap<QString, QLineEdit*> citation_;
    QLabel* citationPageCaption_ = nullptr;
    QLabel* citationPage_ = nullptr;
    [[nodiscard]] QMap<QString, QString> citationDraft() const;
    void fillCitation(const QMap<QString, QString>& attributes, bool onlyUnedited, const QMap<QString, QString>& was);
    QMap<QString, World> worlds_;
    bool worldsKnown_ = false;
    bool awayLatched_ = false;   // a save or read refused toward an absent world, until a read finds it back
    QString imagePath_;
    QImage image_;
    bool saving_ = false;
    bool imageWorking_ = false;
    bool graphWorking_ = false;
    bool closeAfterSave_ = false;
    bool closing_ = false;
    int readGeneration_ = 0;
    QList<Entry> back_;   // C3: the objects behind this one, oldest first
    std::function<void()> afterSave_;   // a navigation waiting on the save its prompt asked for
    QList<Edge> edges_;
    bool edgesRead_ = false;
    QTimer* sourceClick_ = nullptr;   // a click waits out the double-click interval: one gesture, one action
    QString sourceClickEdge_;

    QStackedWidget* pages_ = nullptr;
    QWidget* messagePage_ = nullptr;
    QLabel* messageTitle_ = nullptr;
    QLabel* messageText_ = nullptr;
    QLabel* messageHint_ = nullptr;
    QLabel* readOnlyBanner_ = nullptr;
    QLabel* atRiskBanner_ = nullptr;
    QLineEdit* name_ = nullptr;
    QLineEdit* subtitle_ = nullptr;
    QPlainTextEdit* notes_ = nullptr;
    QWidget* tagChips_ = nullptr;
    QLabel* noTags_ = nullptr;
    QLineEdit* newTag_ = nullptr;
    QLabel* saveError_ = nullptr;
    QLabel* imagePreview_ = nullptr;
    QLabel* imageMessage_ = nullptr;
    QLabel* imageFile_ = nullptr;
    QPushButton* imageAdd_ = nullptr;
    QPushButton* imageExisting_ = nullptr;
    QPushButton* imageRemove_ = nullptr;
    QLabel* imageError_ = nullptr;
    QPushButton* cite_ = nullptr;
    QPushButton* newSource_ = nullptr;
    QListWidget* sources_ = nullptr;
    QLabel* noSources_ = nullptr;
    QLabel* sourcesError_ = nullptr;
    QPushButton* relate_ = nullptr;
    QTreeWidget* relations_ = nullptr;
    QLabel* noRelations_ = nullptr;
    QLabel* relationsError_ = nullptr;
    QAction* backAction_ = nullptr;
    QLabel* metadata_ = nullptr;
    QLabel* status_ = nullptr;
    QAction* saveAction_ = nullptr;
    QAction* revertAction_ = nullptr;
    QAction* closeAction_ = nullptr;
    QToolButton* more_ = nullptr;
};
