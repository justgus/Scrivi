#!/usr/bin/env bash
# EP-040 SP-137 / T-0548 — guard the [I-0245] layout-convergence class.
#
# ⚠️ WHY THIS EXISTS. [I-0245] CRASHED THE APP, and nothing caught it:
# `xcodebuild test` 132/132 and `ctest` 626/626 stayed GREEN through FOUR failed
# fix attempts. The exception is not a Swift error and no suite can assert on it:
#
#   NSGenericException: The window has been marked as needing another Update
#   Constraints in Window pass, but it has already had more Update Constraints in
#   Window passes than there are views in the window.
#
# ⚠️ THE MECHANISM, in the user's words: *"It cannot resolve the widths because
# showing the detail sheet requires that one or more views are now smaller than
# their minimum. It adjusts one, which pushes the other past its minimum, it
# adjusts and pushes the first back again."* D1-E had made the Object Detail pane
# an `HStack` SIBLING of the manuscript, so THREE views — manuscript · pane ·
# `.inspector` column ([SP-136]) — competed for a width that could not satisfy all
# three minimums. AppKit looped until it gave up.
#
# ⚠️ WHAT IT CHECKS — THE SHAPE, NOT THE PROPERTY. `manuscriptDetail`'s top-level
# `HStack` is the contended axis. A flexible-width view added as a SIBLING there is
# a new claimant on a width budget that [I-0245] proved is already spent. This
# guard fails on one.
#
# ⛔ WHAT IT CANNOT DO, said plainly. It is a grep over ONE view's shape. It does
# not know about minimum widths, does not evaluate SwiftUI layout, and will not
# catch the same defect built in another file. That is WHY Q4 (user, 2026-09-23)
# ruled BOTH this AND a debug assertion: this half runs in CI on every push and
# guards the shape; the assertion catches the general case at runtime, on any
# surface, and only in a debug build. ⛔ NEITHER ALONE IS THE GUARD.
#
# ⚠️ A SHEET IS THE SAFE ANSWER, and is what [I-0245] shipped: presented ABOVE the
# window, it takes part in no width negotiation, so the ring cannot form.
#
# Run: scripts/check-layout-convergence.sh   (exit 1 on violation)

set -uo pipefail
cd "$(dirname "$0")/.."

status=0
FILE="Scrivi/Views/EditorView.swift"

# ⚠️ THE ALLOW MARKER, same contract as `check-package-boundary.sh`: the reason
# travels WITH the code, not in a line number in this file. ⛔ A bare marker is
# rejected below — an unexamined exception is how the class returns.
MARKER='layout-ok:'

if [ ! -f "$FILE" ]; then
  echo "❌ $FILE not found — this guard is anchored to it by name."
  echo "If the view moved, MOVE THE GUARD WITH IT. ⛔ Do not delete it: [I-0245]"
  echo "crashed the app and no test suite saw it."
  exit 1
fi

# ---------------------------------------------------------------------------
# Extract `manuscriptDetail`'s body
# ---------------------------------------------------------------------------
# ⚠️ From its declaration to the next declaration at the same indent. ⛔ Not the
# whole file: `EditorView` has other `HStack`s (the bars, the warning strip) and
# they are not on the contended axis.
body=$(awk '
  /private var manuscriptDetail: some View \{/ { inside = 1 }
  inside { print }
  inside && /^    \/\/ MARK:/ && !/manuscriptDetail/ { exit }
' "$FILE")

if [ -z "$body" ]; then
  echo "❌ Could not find 'manuscriptDetail' in \$FILE."
  echo "⚠️ If it was RENAMED, rename it here too. ⛔ A guard that silently matches"
  echo "nothing is worse than no guard: it reports success forever."
  exit 1
fi

# ⚠️ Confirm the shape we are guarding is still the shape that exists. If the
# top-level `HStack` became something else, the assumptions above no longer hold
# and the guard must be re-derived rather than left reporting green.
if ! printf '%s\n' "$body" | grep -qE '^\s*HStack\(spacing: 0\) \{'; then
  echo "❌ 'manuscriptDetail' no longer opens with its top-level HStack."
  echo "⚠️ This guard assumes that stack IS the contended width axis ([I-0245])."
  echo "Re-derive it against the new shape — ⛔ do not just delete it."
  exit 1
fi

# ---------------------------------------------------------------------------
# A flexible-width sibling on the contended axis
# ---------------------------------------------------------------------------
# ⚠️ WHAT MAKES A CLAIMANT. `maxWidth: .infinity` says "give me whatever is left";
# `minWidth:`/`idealWidth:` state a floor AppKit must honour. Either, on a view
# that is a sibling of the manuscript, is a new demand on a spent budget.
# ⚠️ `.inspector` is included because [SP-136] made the Inspector a REAL column —
# a SECOND one would contend with it exactly as D1-E's pane did.
FLEX_PATTERN='maxWidth: \.infinity|minWidth:|idealWidth:|\.inspector\('

# ⚠️ The manuscript's OWN `.frame(maxWidth: .infinity, maxHeight: .infinity)` is
# the legitimate one — it is the view that SHOULD absorb the remaining width. It
# carries the marker at its site.
hits=$(printf '%s\n' "$body" \
  | grep -nE "$FLEX_PATTERN" \
  | grep -vE '^\s*[0-9]+:\s*(//|///|\*)' \
  | grep -v "$MARKER" \
  || true)

if [ -n "$hits" ]; then
  echo "❌ A flexible-width view is competing on manuscriptDetail's width axis."
  echo "$hits"
  echo
  echo "⚠️ [I-0245]: manuscript + pane + .inspector column could not all clear their"
  echo "minimums. AppKit adjusted one, pushed another past its floor, adjusted back,"
  echo "and looped until it threw:"
  echo "  NSGenericException: … more Update Constraints in Window passes than there"
  echo "  are views in the window."
  echo
  echo "⛔ DO NOT answer this by zeroing the minimums. That resolves the arithmetic"
  echo "by letting the manuscript collapse to nothing, and re-opens the same"
  echo "negotiation for every future pane. (Rejected during [I-0245].)"
  echo
  echo "✅ Present it as a .sheet instead — above the window, it takes part in no"
  echo "width negotiation. That is what [I-0245] shipped and [SP-137] Q1 ruled"
  echo "the INTENDED shape."
  echo
  echo "If this view genuinely does not claim width, mark the line:"
  echo "  // layout-ok: <why this cannot contend for width>"
  status=1
fi

# ---------------------------------------------------------------------------
# The marker must carry a REASON
# ---------------------------------------------------------------------------
bare=$(grep -rnE "$MARKER\s*$" Scrivi/ 2>/dev/null || true)
if [ -n "$bare" ]; then
  echo "❌ A 'layout-ok:' marker carries no reason."
  echo "$bare"
  echo
  echo "Say WHY the view cannot contend — e.g."
  echo "  // layout-ok: the manuscript itself; this is the view that absorbs the rest"
  status=1
fi

if [ "$status" -eq 0 ]; then
  echo "✅ Layout convergence clean — no new claimant on the manuscript's width axis."
fi
exit "$status"
