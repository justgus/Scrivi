#!/usr/bin/env bash
# EP-047 S2 (SP-168, T-0597) — guard AC4: the manuscript's type has ONE source, `ManuscriptTypography`.
#
# ⚠️ WHY THIS EXISTS. Before SP-168 the manuscript's font was built in NINE places, four of them hard-coding a system
# font; a typeface read from the project's settings would have vanished on undo, on rebuild or on Replace All
# (EP-047's TRAP). The interop tests drive typing, Return, paste, Replace All and the rebuild — ⛔ but NOT the history
# undo/redo apply path (`applySceneChange`, private, needs live history): a mutation there SURVIVED the suite
# (measured 2026-10-07). This guard is what covers it, and any new site anyone adds.
#
# ⚠️ WHAT IT CHECKS: no manuscript source file but `ManuscriptTypography.swift` constructs a font
# (`NSFont.systemFont(`, `monospacedSystemFont`, `boldSystemFont`, `NSFont(name:`, `NSFontManager`, `NSFont.systemFontSize`).
# A deliberate exception carries `type-source-ok` and its reason on the same line.
#
# Run: scripts/check-manuscript-type-source.sh     (exit 1 on a second source)
set -uo pipefail
cd "$(dirname "$0")/.."
hits=$(grep -n -E 'NSFont\.(systemFont|monospacedSystemFont|boldSystemFont|monospacedDigitSystemFont)\(|NSFont\(name|NSFontManager|NSFont\.systemFontSize' \
    Scrivi/Views/Manuscript*.swift Scrivi/Views/Markdown*.swift \
    | grep -v '^Scrivi/Views/ManuscriptTypography.swift:' | grep -v 'type-source-ok')
if [ -n "$hits" ]; then
    echo "⛔ A second source of manuscript type (EP-047 AC4) — build fonts through ManuscriptTypography:"
    echo "$hits"
    exit 1
fi
echo "✅ Manuscript type has one source (ManuscriptTypography)."
