// glyph_smoke — EP-060 SP-243 (T-0706): the Inkwyn glyphs on Linux.
//
//   §1 every bundled glyph (all three drawings) loads and draws ink at 16, 24 and 32 px;
//   §2 a glyph takes the PALETTE's text colour — dark on a light theme, light on a dark one — and the disabled colour;
//   §3 a tint replaces the palette colour; an unknown name is a null icon; the app icon is bundled;
//   §4 HiDPI: a 2× pixmap is rendered at twice the pixels, not scaled up.
//
// Prints "glyph-ok" when every check passes.
#include "GlyphIcon.hpp"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QImage>
#include <QPalette>
#include <QPixmap>
#include <QSet>

#include <cstdio>

namespace {
int failures = 0;
void check(bool cond, const QString& what)
{
    std::printf("%s %s\n", cond ? "ok  " : "FAIL", qPrintable(what));
    if (!cond) { ++failures; }
}
// The ink's mean colour (alpha-weighted), and whether there is any.
bool ink(const QImage& img, QColor* mean = nullptr)
{
    double r = 0, g = 0, b = 0, a = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const QColor c = QColor::fromRgba(img.pixel(x, y));
            if (c.alpha() == 0) { continue; }
            r += c.redF() * c.alphaF(); g += c.greenF() * c.alphaF(); b += c.blueF() * c.alphaF(); a += c.alphaF();
        }
    }
    if (a == 0) { return false; }
    if (mean != nullptr) { *mean = QColor::fromRgbF(r / a, g / a, b / a); }
    return true;
}
QImage draw(const QIcon& icon, int px, QIcon::Mode mode = QIcon::Normal)
{
    return icon.pixmap(QSize(px, px), mode).toImage().convertToFormat(QImage::Format_ARGB32);
}
}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // ── §1 every glyph ──────────────────────────────────────────────────────────────────────────────────────────────────
    QSet<QString> names;
    for (const QString& f : QDir(QStringLiteral(":/glyphs")).entryList({QStringLiteral("*.svg")})) {
        names.insert(f.section(QLatin1Char('-'), 0, -2));   // inkwyn.menu.view-large.svg → inkwyn.menu.view
    }
    check(names.size() >= 54, QStringLiteral("§1 %1 glyphs bundled").arg(names.size()));
    int blank = 0;
    for (const QString& n : names) {
        check(GlyphIcon::exists(n), QStringLiteral("§1 %1 has all three drawings").arg(n));
        for (int px : {16, 24, 32}) {
            if (!ink(draw(GlyphIcon::icon(n), px))) {
                ++blank;
                check(false, QStringLiteral("§1 %1 draws ink at %2 px").arg(n).arg(px));
            }
        }
    }
    check(blank == 0, QStringLiteral("§1 every glyph draws ink at 16, 24 and 32 px"));

    // ── §2 the palette's colour ─────────────────────────────────────────────────────────────────────────────────────────
    const QIcon warn = GlyphIcon::icon(QStringLiteral("inkwyn.warnings"));
    QPalette light = app.palette();
    light.setColor(QPalette::WindowText, Qt::black);
    light.setColor(QPalette::Disabled, QPalette::WindowText, QColor(150, 150, 150));
    QApplication::setPalette(light);
    QColor c;
    check(ink(draw(warn, 24), &c) && c.lightness() < 40, QStringLiteral("§2 light theme: dark ink (lightness %1)").arg(c.lightness()));
    check(ink(draw(warn, 24, QIcon::Disabled), &c) && qAbs(c.lightness() - 150) < 20,
          QStringLiteral("§2 disabled: the disabled colour (lightness %1)").arg(c.lightness()));
    QPalette dark = light;
    dark.setColor(QPalette::WindowText, Qt::white);
    QApplication::setPalette(dark);
    check(ink(draw(warn, 24), &c) && c.lightness() > 215,
          QStringLiteral("§2 dark theme, SAME icon object: light ink (lightness %1)").arg(c.lightness()));

    // ── §3 tint, unknown, app icon ─────────────────────────────────────────────────────────────────────────────────────
    check(ink(draw(GlyphIcon::icon(QStringLiteral("inkwyn.warnings"), GlyphIcon::Drawing::BySize, QColor(255, 140, 0)), 24), &c)
              && c.red() > 200 && c.blue() < 60,
          QStringLiteral("§3 a tint wins over the palette (%1)").arg(c.name()));
    check(GlyphIcon::icon(QStringLiteral("inkwyn.no.such.glyph")).isNull(), QStringLiteral("§3 an unknown name is a null icon"));
    check(!GlyphIcon::appIcon().isNull() && ink(draw(GlyphIcon::appIcon(), 64)), QStringLiteral("§3 the app icon is bundled"));

    // ── §4 HiDPI ───────────────────────────────────────────────────────────────────────────────────────────────────────
    const QPixmap two = warn.pixmap(QSize(24, 24), 2.0);
    check(two.width() == 48 && two.devicePixelRatio() == 2.0,
          QStringLiteral("§4 a 2× pixmap is %1 px at ratio %2").arg(two.width()).arg(two.devicePixelRatio()));

    if (failures == 0) { std::printf("glyph-ok\n"); }
    else { std::printf("%d failure(s)\n", failures); }
    return failures == 0 ? 0 : 1;
}
