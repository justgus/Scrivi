#include "SceneInspector.hpp"

#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

// The panel lives in EditorShell's QSplitter, so the minimum is a hard floor and
// the default is applied by the splitter's initial sizes. Narrower than Apple's
// 240/280 by user preference (2026-07-22): 120 min / 200 default keeps the writing
// surface wider.
namespace {
constexpr int kMinWidth     = 120;
constexpr int kDefaultWidth = 200;
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
    // Stub content — the exact placeholder Apple's SceneEntitiesTabView shows
    // (EP-014): a headline title, a muted "No entities yet." empty state, and a
    // DISABLED "Add Entity" button. No project data until a future EP-024 sprint.
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* title = new QLabel(tr("Scene Entities"), tab);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(title);

    layout->addStretch(1);

    auto* empty = new QLabel(tr("No entities yet."), tab);
    empty->setAlignment(Qt::AlignCenter);
    // Muted, like SwiftUI's .foregroundStyle(.secondary).
    empty->setEnabled(false);
    layout->addWidget(empty);

    auto* addEntity = new QPushButton(tr("Add Entity"), tab);
    addEntity->setEnabled(false);   // placeholder — no add flow yet
    layout->addWidget(addEntity, 0, Qt::AlignCenter);

    layout->addStretch(1);

    return tab;
}
