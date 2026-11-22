#pragma once

// [SP-235] EP-059 B1–B4 — the object detail view, as Apple's `ObjectDetailSheet` + `ObjectImageSection`.
//
// A window-modal dialog over the project window (Q1): Name, Subtitle, Notes, Tags, the image and the metadata. Save patches the
// object as it is on disk now, merged per field (I-0155); a re-read keeps what the writer edited (I-0165b); an object whose world
// is away is shown read-only and explained, never written (R9); a world going away while open loses nothing (I-0165, T-0450).
// ⚠️ It reads nothing from its host (Apple's S8): everything arrives as a parameter, and every read and write runs on a worker.

#include <QDialog>
#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QStringList>

#include <functional>

class QAction;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
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

public slots:
    void save();
    void revert();
    void requestClose();   // every exit: ✕, Esc, the title bar — asks when there are unsaved changes (T-0452)

signals:
    void saved();   // a write landed: the host refreshes the Inspector (I-0155)

protected:
    void reject() override;
    void closeEvent(QCloseEvent* event) override;

private:
    struct Detail {
        QJsonObject json;
        QString displayName, subtitle, notes, modifiedAt, imageAssetID;
        QStringList tags;
    };
    struct World {
        QString name, status, reason;
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

    ScriviBridge* bridge_;
    QString root_;
    QString projectID_;
    Entry entry_;

    bool hasDetail_ = false;
    Detail detail_;
    QStringList tags_;
    QMap<QString, World> worlds_;
    bool worldsKnown_ = false;
    bool awayLatched_ = false;   // a save or read refused toward an absent world, until a read finds it back
    QString imagePath_;
    QImage image_;
    bool saving_ = false;
    bool imageWorking_ = false;
    bool closeAfterSave_ = false;
    bool closing_ = false;
    int readGeneration_ = 0;

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
    QLabel* metadata_ = nullptr;
    QLabel* status_ = nullptr;
    QAction* saveAction_ = nullptr;
    QAction* revertAction_ = nullptr;
    QAction* closeAction_ = nullptr;
};
