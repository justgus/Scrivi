#pragma once

#include <QList>

#include <functional>

#include "ManuscriptEditor.hpp"

class QAction;
class QMenuBar;
class QObject;

// SP-175 / T-0602 (EP-048 S3b) — THE FORMAT MENU, the Linux shape of Apple's (`ScriviApp.swift`, [SP-163]): Bold, Italic,
// Heading 1–3, Body, Bulleted List, Numbered List, with Ctrl in place of ⌘ (L7). ✅ [SP-175] Q4: it sits between Edit and
// View, so the caller adds it right after the Edit menu.
// ✅ ONE builder, used by the project window AND by the smoke, so the smoke drives the real shortcuts.
// Returns the actions (the window disables them with no editor).
QList<QAction*> addFormatMenu(QMenuBar* bar, QObject* context, std::function<void(ManuscriptEditor::Format)> apply);
