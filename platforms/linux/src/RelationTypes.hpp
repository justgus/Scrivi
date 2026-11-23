#pragma once

// [SP-238] (EP-059 C5) — a writer's relationship types on Linux, as the Mac's ([SP-237], `RelationTypeForm.swift`):
// the form (RelationTypeDialog) and Project Settings ▸ Relationship Types (RelationTypesBox).
//
// ⚠️ The rules are the core's (`RelationTypeStore::upsert`): the form only greys what the core would refuse, and says why.
// A type that edges use keeps its kinds and Same both ways; its labels may always change. The code is never shown — an
// empty code asks the core for a new, unique one.

#include <QDialog>
#include <QGroupBox>
#include <QVariantMap>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class ScriviBridge;

class RelationTypeDialog : public QDialog
{
    Q_OBJECT
public:
    // `editing`: a row of `listRelationTypes`, or empty for a new type. `initialSourceKind`: a new type's From ("" = Any).
    RelationTypeDialog(ScriviBridge* bridge, QString projectRootPath, QVariantMap editing, const QString& initialSourceKind,
                       QWidget* parent);
    QString savedCode() const { return savedCode_; }   // valid once accepted

    // For smokes.
    QLineEdit* forwardField() const { return forward_; }
    QLineEdit* inverseField() const { return inverse_; }
    QCheckBox* symmetricBox() const { return symmetric_; }
    QComboBox* fromBox() const { return from_; }
    QComboBox* toBox() const { return to_; }
    QPushButton* saveButton() const { return save_; }

private:
    void refresh();
    void save();

    ScriviBridge* bridge_;
    QString root_;
    QVariantMap editing_;
    QString savedCode_;
    QLineEdit* forward_ = nullptr;
    QLineEdit* inverse_ = nullptr;
    QLabel* inverseLabel_ = nullptr;
    QCheckBox* symmetric_ = nullptr;
    QLabel* labelsFooter_ = nullptr;
    QLabel* fromLabel_ = nullptr;
    QComboBox* from_ = nullptr;
    QLabel* toLabel_ = nullptr;
    QComboBox* to_ = nullptr;
    QLabel* error_ = nullptr;
    QPushButton* save_ = nullptr;
    bool saving_ = false;
};

class RelationTypesBox : public QGroupBox
{
    Q_OBJECT
public:
    RelationTypesBox(ScriviBridge* bridge, QString projectRootPath, QWidget* parent);

    // For smokes: the dialog Edit… or New Relationship Type… opened, while it is up.
    RelationTypeDialog* openForm() const { return form_; }

private:
    void load();
    void showForm(const QVariantMap& type);

    ScriviBridge* bridge_;
    QString root_;
    QVBoxLayout* rows_ = nullptr;
    QLabel* error_ = nullptr;
    RelationTypeDialog* form_ = nullptr;
};
