#!/usr/bin/env bash
# EP-041 SP-149 / T-0541 — guard the ScriviCore boundary ([I-0197]).
#
# ⚠️ WHY THIS EXISTS. CLAUDE.md states the architecture's central rule without
# qualification: "No backend logic is reimplemented in Swift. Swift is responsible
# for UI only." [I-0197] found 35 places breaking it. The class does not announce
# itself — an app that reads a project file directly WORKS, right up until two
# platforms disagree about the same schema.
#
# ⚠️ IT ALREADY HAPPENED. [I-0215]: Apple's `InspectorLayoutStore` DROPPED keys it
# did not understand while Linux's PRESERVED them — same file, same schema, two
# behaviours, and Linux was the one that had it right. Nothing caught it; a writer
# would have found her card layout gone after moving between machines.
#
# ⚠️ WHAT IT CHECKS. Reads and writes, NOT existence checks. `fileExists` is
# [I-0197] Class C and was RULED legitimate on 2026-09-18 (T-0508) — including it
# would re-litigate a closed decision and inflate the allow-list eightfold.
# (User ruling Q1, 2026-09-21.)
#
# ⚠️ IT IS A TRIPWIRE, NOT A TYPE SYSTEM. Its job is to make the class LOUD when it
# returns — not to prove absence. A determined bypass can evade a grep; an
# accidental one, which is how all of them arrived, cannot.
#
# Run: scripts/check-package-boundary.sh     (exit 1 on violation)

set -uo pipefail
cd "$(dirname "$0")/.."

status=0

# ⚠️ THE ALLOW MARKER. A legitimate site carries `// boundary-ok: <reason>` on the
# line itself. ⛔ A marker with no reason after the colon is a silent exception —
# `check-textkit2.sh` says it plainly and it is right — so the check below rejects
# a bare one.
MARKER='boundary-ok:'

# ---------------------------------------------------------------------------
# Apple — Swift reading/writing bytes
# ---------------------------------------------------------------------------
# ⚠️ `String(contentsOf:)` and `FileHandle` are here DELIBERATELY. The first draft
# of this pattern had only `.write(to:` and `Data(contentsOf:)`, and re-measuring
# with the additions immediately surfaced a site the three-term version missed
# (TimelineStripView.swift:1977). A read-only bypass is still a bypass.
APPLE_PATTERN='\.write\(to:|Data\(contentsOf:|String\(contentsOf:|FileHandle'

# ⚠️ ALLOW-LIST. EACH ENTRY NEEDS A REASON. An unexamined exception is how this
# class returns — `check-textkit2.sh` says the same thing, and it is right.
#
#   TimelineStripView.swift:1977  — timeline IMPORT via NSOpenPanel.
#   TimelineStripView.swift:2004  — timeline EXPORT via NSSavePanel.
#       The WRITER picks the path and it is OUTSIDE any project package: moving
#       bytes out of the app is not a boundary violation.
#
# ⚠️ ALLOW-LISTED BY SITE, NOT BY FILE. A whole-file exemption would blind the
# guard to a real package write ANYWHERE in that file — and TimelineStripView is
# precisely where [SP-129]/[T-0502] had to remove one (`readImportedTimelineFile`
# walked `objects/imported-timelines/` directly). Exempting the file that already
# earned this defect once would be the worst possible place to be imprecise.
#
# ⚠️ THE MARKER IS A TRAILING `// boundary-ok:` COMMENT ON THE LINE ITSELF, with
# its reason. ⛔ Line NUMBERS were tried first and were wrong within minutes: the
# very next edit (a four-line tombstone) shifted two legitimate sites and the guard
# went red on them. A number in this file is a second copy of a fact the source
# already knows — exactly the staleness the project's P7 rule is about.
# ✅ A marker moves WITH the code it excuses, and it is visible AT the site to the
# next person who reads it, not only here.

apple_hits=$(grep -rnE "$APPLE_PATTERN" Scrivi/ 2>/dev/null \
  | grep -v '^Scrivi/Tests/' \
  | grep -vE '^[^:]+:[0-9]+:\s*(//|///|\*)' \
  | grep -v "$MARKER" \
  || true)

if [ -n "$apple_hits" ]; then
  echo "❌ [Apple] Swift is reading or writing files directly."
  echo "$apple_hits"
  echo
  echo "Project data belongs to ScriviCore. Call a scrivi_* endpoint through"
  echo "ScriviEngine instead of touching the package:"
  echo "  reading a schema  → the endpoint that projects it"
  echo "  writing a schema  → the endpoint that owns it (the core does the"
  echo "                      atomic temp+rename, and the repair)"
  echo
  echo "If the path is genuinely OUTSIDE a project package (a user-chosen export"
  echo "via NSSavePanel, say), add it to APPLE_ALLOW in this script WITH A REASON."
  status=1
fi

# ---------------------------------------------------------------------------
# Linux — Qt reading/writing bytes
# ---------------------------------------------------------------------------
# ⚠️ THE GUARD COVERS BOTH APP LAYERS (user ruling Q3, 2026-09-21). [I-0197] is
# filed [Apple], but the DUPLICATION it exists to stop is cross-platform — and
# EP-041's scope note says the guard "is what stops Windows re-earning it".
# ⚠️ ONE script, TWO pattern sets: a single place to state the rule.
LINUX_PATTERN='QFile |QFile\(|QSaveFile |QSaveFile\('

#   EditorShell.cpp:2560      — timeline EXPORT via QFileDialog (writer-chosen).
#   EditorShell.cpp:2665      — timeline IMPORT via QFileDialog (writer-chosen).
#       Mirror Apple's panels above; outside any project package.
#   RecentsStore.cpp:70, :122 — recents.json under appSupportRoot. NOT a project
#       package: recents are an app-side concern with no scrivi_* endpoint, and
#       the store's own header says so.
#
# ⚠️ BY SITE, NOT BY FILE — see the Apple note. EditorShell.cpp is 2,700+ lines and
# is exactly where [I-0241]'s disk walk lived; a whole-file exemption there would
# have allow-listed the defect this Epic just removed.

linux_hits=$(grep -rnE "$LINUX_PATTERN" platforms/linux/src/ 2>/dev/null \
  | grep -vE '^[^:]+:[0-9]+:\s*(//|\*)' \
  | grep -v "$MARKER" \
  || true)

if [ -n "$linux_hits" ]; then
  echo "❌ [Linux] Qt is reading or writing files directly."
  echo "$linux_hits"
  echo
  echo "Project data belongs to ScriviCore. Call through ScriviBridge instead."
  echo "⚠️ Do not answer a missing endpoint by parsing the package yourself — that"
  echo "is how [I-0241] happened (EditorShell listed and parsed EVERY historical"
  echo "event file to recover one field the projection dropped). EXTEND THE CORE."
  echo
  echo "If the path is outside a project package (a QFileDialog export, or"
  echo "appSupportRoot), add it to LINUX_ALLOW in this script WITH A REASON."
  status=1
fi

# ---------------------------------------------------------------------------
# The marker must carry a REASON
# ---------------------------------------------------------------------------
# ⚠️ "An unexamined exception is how this returns" — check-textkit2.sh, and it is
# right. A bare `// boundary-ok:` silences the guard while explaining nothing, so
# it is itself a failure. ⛔ This is what stops the allow-list becoming a dumping
# ground, which is the risk [SP-143]'s plan names first.
bare=$(grep -rnE "$MARKER\s*$" Scrivi/ platforms/linux/src/ 2>/dev/null || true)
if [ -n "$bare" ]; then
  echo "❌ A 'boundary-ok:' marker carries no reason."
  echo "$bare"
  echo
  echo "Say WHY the site is legitimate — e.g."
  echo "  // boundary-ok: NSSavePanel export, writer-chosen path outside any package"
  status=1
fi

if [ "$status" -eq 0 ]; then
  echo "✅ Boundary clean — no app-side package reads or writes."
fi
exit "$status"
