#!/usr/bin/env bash
# escape_smoke.sh — EP-049 (SP-160, T-0586): Linux writes Apple's manuscript format.
#
# Runs the scrivi_linux_escape_smoke harness against the SHARED corpus — the same cases Apple's
# interop suite runs — through the real ManuscriptEditor. Offscreen; no project on disk.
#
# Usage: escape_smoke.sh <path-to-harness-binary> [path-to-manuscript_format_corpus.json]
set -euo pipefail

BIN="${1:?usage: escape_smoke.sh <harness-binary> <corpus.json>}"
# The corpus defaults to the repo copy, so the generic smoke loop (deploy-to-rig.sh --test) can run it.
CORPUS="${2:-$(cd "$(dirname "$0")/../../.." && pwd)/ScriviCore/tests/fixtures/manuscript_format_corpus.json}"

export QT_QPA_PLATFORM=offscreen

echo "== manuscript storage format (shared corpus + Linux-only gestures) =="
OUT="$("$BIN" "$CORPUS")"
if [ "$OUT" != "escape-ok" ]; then
    echo "FAIL: harness did not report escape-ok (got: '$OUT')"
    exit 1
fi

echo "PASS: Linux stores the same bytes Apple does for every corpus gesture."
