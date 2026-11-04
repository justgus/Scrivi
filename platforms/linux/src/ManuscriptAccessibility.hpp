#pragma once

// EP-050 S4 ([SP-203], T-0659) — THE MANUSCRIPT'S ACCESSIBILITY ON LINUX: Orca reads the page AT REST (Apple's A1).
//
// ⛔ Measured ([SP-202]): Qt's own `QPlainTextEdit` interface hands AT-SPI the STORED manuscript — `\.`, `##`, `**`.
// ✅ The Linux shape of Apple's `ManuscriptAccessibility.swift`: EVERY text member answers in PAGE positions through the
// ONE map (`ManuscriptFind::PresentedText`, shared with Find — Apple's Q3); lines are Qt's visual lines, mapped in and out.
// ✅ A caret Orca sets goes through Scrivi's caret rules (`ManuscriptEditor::placeCaret`, Apple AC2).
// ✅ Qt's own text events carry STORED positions; `install()` translates them before AT-SPI sees them.
// ⛔ No editable-text interface: Orca reads and moves; typing reaches the editor as keys.

namespace ManuscriptAccessibility {

// Install once, after the QApplication exists: the interface factory and the event translation.
void install();

}   // namespace ManuscriptAccessibility
