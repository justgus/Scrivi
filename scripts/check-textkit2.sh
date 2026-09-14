#!/usr/bin/env bash
# EP-039 T-0527 — guard the TextKit 2 migration.
#
# ⚠️ WHY THIS EXISTS. An `NSTextView` starts on TextKit 2, which lays out only the
# VIEWPORT. Reading the TextKit-1 `.layoutManager` property makes AppKit PERMANENTLY
# DOWNGRADE that view to TextKit 1, which lays out the WHOLE DOCUMENT.
#
# ✅ MEASURED on the real 1,831,770-char manuscript:
#     initial layout   TextKit 1 = 272.8 ms  →  TextKit 2 = 0.9 ms
#     window resize    TextKit 1 = 222.5 ms  →  viewport-bounded
#     rebuildStorage   TextKit 1 = 270.0 ms  →  viewport-bounded
#
# ⚠️ THE DOWNGRADE IS ONE LINE AND IT IS SILENT — nothing warns, nothing crashes, the app
# just gets slow. ⚠️ It had already happened THREE times before this guard existed, in TWO
# different files, and one of them (ForkPopover) fired on every call from inside an
# `if let` whose branch was rarely taken.
#
# Run: scripts/check-textkit2.sh     (exit 1 on violation)

set -uo pipefail
cd "$(dirname "$0")/.."

# TextKit 1 API that downgrades or is AppKit-only. `textLayoutManager` is TextKit 2 and
# must NOT match, hence the negative lookbehind via grep -v below.
PATTERN='\.layoutManager|NSLayoutManager|NSTextAttachmentCell'

# ⚠️ ALLOW-LIST. Each entry needs a REASON. An unexamined exception is how this returns.
#   (none today — every site was migrated in SP-133)
ALLOW='^$'

hits=$(grep -rnE "$PATTERN" --include="*.swift" Scrivi/ 2>/dev/null \
  | grep -v 'textLayoutManager' \
  | grep -vE '^\s*[^:]+:[0-9]+:\s*//' \
  | grep -vE "$ALLOW" \
  | grep -v 'check-textkit2' || true)

if [ -n "$hits" ]; then
  echo "❌ TextKit 1 usage found — this DOWNGRADES the text view and makes layout O(document)."
  echo "$hits"
  echo
  echo "Use TextKit 2 instead:"
  echo "  point → index : tv.textLayoutManager?.textLayoutFragment(for:)"
  echo "  index → rect  : NSTextLayoutFragment.layoutFragmentFrame"
  echo "  attachments   : override NSTextAttachment.attachmentBounds / image(for:)"
  echo
  echo "If a use is genuinely unavoidable, add it to ALLOW in this script WITH A REASON."
  exit 1
fi

echo "✅ TextKit 2 clean — no downgrading API in Scrivi/."
