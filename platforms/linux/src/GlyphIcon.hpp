#pragma once

#include <QColor>
#include <QIcon>
#include <QSize>
#include <QString>

class QToolButton;
class QWidget;

// EP-060 SP-243 (Q2): an Inkwyn glyph as a QIcon. The glyphs are the SVGs scripts/make-glyph-symbols.swift writes to
// resources/glyphs (the same drawings as the Apple symbols — one pipeline), bundled at :/glyphs/<name>-<size>.svg.
//
// The icon draws itself at each paint: the right drawing for the size, aspect-fit, TINTED with the application palette's text
// colour (the disabled one when disabled) — so a light/dark switch and HiDPI need no code at the call site.
namespace GlyphIcon {

enum class Drawing { BySize, Small, Medium, Large };   // BySize: Small ≤ 16 px, Medium ≤ 24 px, Large above

// `name` is the Apple symbol's, e.g. "inkwyn.warnings". An unknown name gives a null icon. `tint` replaces the palette's text
// colour (e.g. ThemeColours::attention for a warning) — fixed at creation, so it is for colours already derived from a palette.
QIcon icon(const QString& name, Drawing drawing = Drawing::BySize, const QColor& tint = QColor());

// A glyph that sits in a layout like a label: a button that takes no clicks and no focus (an icon is drawn at each paint, so it
// follows a theme switch, where a QLabel pixmap would not).
QToolButton* mark(const QString& name, const QSize& size, QWidget* parent, const QColor& tint = QColor());

// True when all three drawings of `name` are bundled.
bool exists(const QString& name);

// The Inkwyn app icon (the window icon).
QIcon appIcon();

}   // namespace GlyphIcon
