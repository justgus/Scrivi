#pragma once

#include <QPointer>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include "HistoryCapture.hpp"

class QCheckBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QToolButton;

// HistorySection — the Scene Inspector's History, on Linux (EP-026 S3, [SP-204] / T-0662). ✅ Apple's `HistoryCard`
// (`Scrivi/Views/Inspector/HistoryCard.swift`): the windowed tree (the primary line newest first, then the branches), the
// "This scene only" scope with its honest count, the caret's event in bold, a click on a branch re-primaries its fork (no
// redo), and a stale-branch BADGE ([I-0108]: purge is Project ▸ Purge Stale History Branches…, never here).
// ✅ [SP-204] Q1 (user, 2026-11-05): a fourth section at the foot of the Writing tab, collapsible, closed by default.
// ✅ [I-0331]: the tree is the PROJECT's — fetched on a history change only, off the GUI thread; a scene change or a caret
// move only re-filters it.
class HistorySection : public QWidget
{
    Q_OBJECT

public:
    explicit HistorySection(QWidget* parent = nullptr);

    // The open project's history (nullptr: none open).
    void setHistory(HistoryCapture* history);
    // The scene in view, and the caret as a scene-local UTF-8 byte (-1: the caret is not in that scene).
    void setScene(const QString& sceneID);
    void setCaret(const QString& sceneID, int caretByte);
    [[nodiscard]] bool isExpanded() const;

    // For the smokes: the rows as shown, and a click on one.
    [[nodiscard]] QListWidget* rows() const { return list_; }
    [[nodiscard]] int fetches() const { return generation_; }   // tree fetches started ([I-0331]: never per scene)
    [[nodiscard]] QToolButton* header() const { return header_; }

private:
    void reload();
    void render();
    void select(QListWidgetItem* item);
    void markCaretRow();

    QPointer<HistoryCapture> history_;
    QString sceneID_;
    QString caretScene_;
    int caretByte_ = -1;
    QVariantMap tree_;
    int staleCount_ = 0;
    bool loadFailed_ = false;
    bool loaded_ = false;
    int generation_ = 0;   // a fetch that a newer one overtook is dropped

    QToolButton* header_ = nullptr;
    QWidget* body_ = nullptr;
    QCheckBox* sceneOnly_ = nullptr;
    QLabel* count_ = nullptr;
    QListWidget* list_ = nullptr;
    QLabel* notice_ = nullptr;
    QLabel* truncated_ = nullptr;
    QLabel* stale_ = nullptr;
};
