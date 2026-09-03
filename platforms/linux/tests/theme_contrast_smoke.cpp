// theme_contrast_smoke — EP-035 / SP-127 (I-0186): secondary text must be
// LEGIBLE in the themes people actually run.
//
// ⚠️ THE DEFECT THIS PINS. Both Linux surfaces styled secondary text with named
// Qt palette roles that carry no contrast guarantee — `palette(mid)` (a
// STRUCTURAL role for borders) and `palette(link-visited)`. Measured on the rig:
//
//     Yaru-dark   Mid #2f2f2f on Window #2a2a2a  →  1.07:1
//     Yaru light  Mid #ffffff on Window #fcfcfc  →  1.03:1
//
// ⚠️ The world PATH was invisible in BOTH real themes. The user found it in dark
// mode; light was no better.
//
// ⚠️ WHY IT SURVIVED EVERY EXISTING CHECK — the part worth remembering. Qt's
// NO-THEME fallback renders `Mid` as a pleasant #b8b8b8 grey. Offscreen smokes
// and headless screenshots use that fallback, so the automated suite and a
// screenshot both showed a readable path that no real user ever saw.
// ⚠️ A HEADLESS RENDER IS NOT EVIDENCE ABOUT COLOUR unless it loads a real theme.
//
// So this smoke asserts contrast against the palettes the DERIVATION produces,
// and the harness runs it under a REAL GTK theme, both polarities.
//
//   argv[1] = optional label for the theme under test (for the failure message)
//
// Exit 0 on success; non-zero with a FAIL line naming the ratio.

#include <QApplication>
#include <QColor>
#include <QPalette>

#include <cmath>
#include <cstdio>

#include "ThemeColours.hpp"

namespace {

// WCAG relative luminance / contrast ratio.
double relLuminance(const QColor& c)
{
    auto ch = [](double v) {
        v /= 255.0;
        return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * ch(c.red()) + 0.7152 * ch(c.green()) + 0.0722 * ch(c.blue());
}

double contrast(const QColor& a, const QColor& b)
{
    double l1 = relLuminance(a);
    double l2 = relLuminance(b);
    if (l1 < l2) { std::swap(l1, l2); }
    return (l1 + 0.05) / (l2 + 0.05);
}

// WCAG AA for body text.
constexpr double kMinAA = 4.5;

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    const char* themeLabel = (argc > 1) ? argv[1] : "(unnamed theme)";

    const QPalette pal = app.palette();
    const QColor   bg  = pal.color(QPalette::Active, QPalette::Window);

    struct Check {
        const char* name;
        QColor      colour;
    } checks[] = {
        {"deemphasised", ThemeColours::deemphasised(pal)},
        {"attention",    ThemeColours::attention(pal)},
    };

    int failures = 0;
    for (const auto& c : checks) {
        const double r = contrast(c.colour, bg);
        std::printf("  %-13s #%02x%02x%02x on #%02x%02x%02x  %.2f:1  %s\n",
                    c.name, c.colour.red(), c.colour.green(), c.colour.blue(),
                    bg.red(), bg.green(), bg.blue(), r, r >= kMinAA ? "ok" : "FAIL");
        if (r < kMinAA) {
            std::fprintf(stderr,
                         "FAIL: %s text is %.2f:1 against the window in %s "
                         "(WCAG AA needs %.1f:1)\n",
                         c.name, r, themeLabel, kMinAA);
            ++failures;
        }
    }

    // ⚠️ And the roles that CAUSED this must never come back as text colours.
    // If a future edit reaches for them again, say so with the number.
    const QColor mid = pal.color(QPalette::Active, QPalette::Mid);
    if (contrast(mid, bg) >= kMinAA) {
        std::printf("  note: palette(Mid) happens to pass here (%.2f:1) — "
                    "it is still NOT a text role\n", contrast(mid, bg));
    }

    return failures == 0 ? 0 : 1;
}
