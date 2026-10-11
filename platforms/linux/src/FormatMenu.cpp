#include "FormatMenu.hpp"

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>

QList<QAction*> addFormatMenu(QMenuBar* bar, QObject* context, std::function<void(ManuscriptEditor::Format)> apply)
{
    using F = ManuscriptEditor::Format;
    // Apple: ⌘B ⌘I · ⌥⌘1/2/3 · ⌥⌘0 · ⌥⌘L · ⌥⌘N. ✅ Measured on the rig (SP-175): GNOME binds none of Ctrl+Alt+0–3, L, N.
    struct Item { const char* title; QKeySequence keys; F format; bool separatorBefore; };
    const Item items[] = {
        {QT_TRANSLATE_NOOP("FormatMenu", "Bold"), QKeySequence(Qt::CTRL | Qt::Key_B), F::Bold, false},
        {QT_TRANSLATE_NOOP("FormatMenu", "Italic"), QKeySequence(Qt::CTRL | Qt::Key_I), F::Italic, false},
        {QT_TRANSLATE_NOOP("FormatMenu", "Heading 1"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_1), F::Heading1, true},
        {QT_TRANSLATE_NOOP("FormatMenu", "Heading 2"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_2), F::Heading2, false},
        {QT_TRANSLATE_NOOP("FormatMenu", "Heading 3"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_3), F::Heading3, false},
        {QT_TRANSLATE_NOOP("FormatMenu", "Body"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_0), F::Body, false},
        {QT_TRANSLATE_NOOP("FormatMenu", "Bulleted List"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_L), F::BulletList, true},
        {QT_TRANSLATE_NOOP("FormatMenu", "Numbered List"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_N), F::NumberedList, false},
    };
    QMenu* menu = bar->addMenu(QMenu::tr("F&ormat"));
    QList<QAction*> actions;
    for (const Item& item : items) {
        if (item.separatorBefore) {
            menu->addSeparator();
        }
        QAction* a = menu->addAction(QMenu::tr(item.title));
        a->setShortcut(item.keys);
        const F format = item.format;
        QObject::connect(a, &QAction::triggered, context, [apply, format]() { apply(format); });
        actions.append(a);
    }
    return actions;
}
