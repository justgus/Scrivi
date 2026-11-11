#pragma once

#include <QDialog>
#include <QGroupBox>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ScriviBridge;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QVBoxLayout;

// EP-058 S3 ([SP-211]) — authorship on Linux, in the Mac's shape and wording ([SP-210]; Apple:
// Scrivi/Views/AuthorshipViews.swift). Design: docs/Scrivi_Authorship_Design_v0_2.md §4.2, §4.4.
// ⛔ An `unnamed` author's displayName is "Scrivi" or a host name: it is shown only as "Unnamed author", never as a name.

// Edit ▸ Settings… (Ctrl+,): the writer's settings, not a project's — one tab per pane ([SP-211] Q1).
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
};

// The pen-name menu: the names this writer writes under, and the default New Project and the questions offer.
class PenNamesPane : public QWidget {
    Q_OBJECT
public:
    explicit PenNamesPane(QWidget* parent = nullptr);
    QListWidget* list() const { return list_; }   // for smokes

private:
    void load();
    void save(QStringList names, QString defaultName);
    QString defaultName_;
    QListWidget* list_ = nullptr;
    QLineEdit* newName_ = nullptr;
    QLabel* error_ = nullptr;
};

// The ordered authors of a project or a world, and this machine's I write as. Each change is written at once.
class AuthorsBox : public QGroupBox {
    Q_OBJECT
public:
    // `rootPath`: the project, or a world's package. For a world, `projectRootPath` is the open project's.
    AuthorsBox(ScriviBridge* bridge, QString rootPath, QString projectRootPath = QString(), QWidget* parent = nullptr);
    QListWidget* list() const { return list_; }   // for smokes

signals:
    // I write as changed, or that author was renamed — the window adopts it.
    void writingAsChanged(const QString& personaID, const QString& displayName);

private:
    void load();
    void write(const QVariantList& edits);
    QVariantList edits() const;   // the list as shown, in order
    void move(int offset);

    ScriviBridge* bridge_;
    QString root_;
    QString projectRoot_;
    QVariantList authors_;
    QString writingAs_;
    bool loading_ = false;
    QListWidget* list_ = nullptr;
    QComboBox* add_ = nullptr;
    QComboBox* writingAsBox_ = nullptr;
    QLabel* error_ = nullptr;
};

// Worlds ▸ Authors… — a world's own byline ([SP-210] Q5: edits to its objects are recorded as the project's author).
class WorldAuthorsDialog : public QDialog {
    Q_OBJECT
public:
    WorldAuthorsDialog(ScriviBridge* bridge, const QString& worldName, const QString& worldPath,
                       const QString& projectRootPath, QWidget* parent = nullptr);
};

// *Who wrote “Title”?* (every author unnamed) and *Which of these authors are you?* (this identity writes as no one).
// ⚠️ It cannot be dismissed without an answer ([SP-210] Q4): no Cancel, and Esc and the close button do nothing.
class AuthorQuestionDialog : public QDialog {
    Q_OBJECT
public:
    AuthorQuestionDialog(ScriviBridge* bridge, QString projectRootPath, const QString& title, QWidget* parent = nullptr);
    void reject() override {}

signals:
    void answered(const QString& personaID, const QString& displayName);

private:
    void rebuild();          // for the core's current question
    void answerNames();
    void choose(const QString& personaID);
    void addSelf();
    void finish(const QVariantMap& authors);
    QComboBox* nameField(const QString& text);

    ScriviBridge* bridge_;
    QString root_;
    QString title_;
    QVariantMap view_;       // the last getAuthors
    QVBoxLayout* body_ = nullptr;
    QList<QPair<QString, QComboBox*>> nameFields_;   // unnamed personaID (empty: a new author) → its field
    QLabel* error_ = nullptr;
};
