#pragma once

#include <QColor>
#include <QPalette>

// ThemeColours — secondary text colours DERIVED from the palette in force
// (EP-035 / SP-127, I-0186).
//
// ## ⚠️ Why this exists: `palette(mid)` is not a text colour
//
// ⚠️ Both Linux surfaces styled their secondary text with Qt stylesheet colour
// roles that are NOT text roles and carry NO contrast guarantee:
//
//   `palette(mid)`          — a STRUCTURAL role: borders, separators, sunken
//                             frames. Nothing requires it to contrast with the
//                             window it sits on.
//   `palette(link-visited)` — a hyperlink role, used here as "warning".
//
// ⚠️ MEASURED ON THE RIG (Yaru / Yaru-dark, the themes actually installed):
//
//     Yaru-dark   Mid #2f2f2f on Window #2a2a2a  →  1.07:1
//     Yaru light  Mid #ffffff on Window #fcfcfc  →  1.03:1
//     both        LinkVisited                     →  MAGENTA #ff00ff
//
// ⚠️ WCAG AA asks 4.5:1 for body text. ⚠️ The world PATH was invisible in BOTH
// real themes — reported by the user in dark mode, but light was no better.
//
// ⚠️ AND IT SURVIVED REVIEW BECAUSE OF HOW IT WAS TESTED: Qt's NO-THEME fallback
// renders `Mid` as a pleasant `#b8b8b8` grey. ⚠️ Offscreen smokes and headless
// screenshots use that fallback, so every automated check — and my own screenshot
// of the row — showed a perfectly readable path that no real user ever saw.
// ⚠️ A headless render is NOT evidence about colour.
//
// ## ✅ The rule
//
// ⚠️ NEVER name a palette role for text colour. ✅ DERIVE it from the two colours
// the theme guarantees are legible against each other — `WindowText` on `Window`
// — so it stays correct in a light theme, a dark theme, and a theme nobody has
// written yet.
namespace ThemeColours {

// De-emphasised body text — paths, hints, empty states. Blended 30% toward the
// background: clearly secondary, still comfortably above AA.
inline QColor deemphasised(const QPalette& pal)
{
    const QColor fg = pal.color(QPalette::Active, QPalette::WindowText);
    const QColor bg = pal.color(QPalette::Active, QPalette::Window);
    constexpr double kBlend = 0.30;
    return QColor::fromRgbF(fg.redF()   * (1.0 - kBlend) + bg.redF()   * kBlend,
                            fg.greenF() * (1.0 - kBlend) + bg.greenF() * kBlend,
                            fg.blueF()  * (1.0 - kBlend) + bg.blueF()  * kBlend);
}

// "Needs attention" text — an unavailable world, a rejected action. A warm amber
// chosen per theme polarity so it reads as emphasis rather than decoration.
//
// ⚠️ Colour is NEVER the only signal: every caller states the problem in a
// SENTENCE. This makes the sentence stand out; it does not carry the meaning.
inline QColor attention(const QPalette& pal)
{
    const QColor fg = pal.color(QPalette::Active, QPalette::WindowText);
    return (fg.lightnessF() > 0.5) ? QColor(0xff, 0xb2, 0x4d)   // dark theme
                                   : QColor(0x9a, 0x53, 0x00);  // light theme
}

// Applies `c` as the widget's text colour. ⚠️ Uses the PALETTE, not a stylesheet:
// a stylesheet colour overrides the theme permanently and does not follow a
// runtime light/dark switch.
template <typename W>
void applyTextColour(W* widget, const QColor& c)
{
    QPalette pp = widget->palette();
    // ⚠️ Set the ACTIVE and INACTIVE groups. A window that loses focus switches to
    // Inactive, and a colour set only on Active silently reverts there.
    for (auto group : {QPalette::Active, QPalette::Inactive}) {
        pp.setColor(group, QPalette::WindowText, c);
        pp.setColor(group, QPalette::Text,       c);
    }
    widget->setPalette(pp);
}

}  // namespace ThemeColours
