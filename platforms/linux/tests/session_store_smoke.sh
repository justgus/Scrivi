#!/usr/bin/env bash
# session_store_smoke.sh — EP-043 / SP-147 (T-0565).
#
# Runs the SessionStore round-trip against a TEMP app-support root, so the check
# can never touch a real writer's session.ini.
#
# ⚠️ It runs the harness TWICE against the SAME root: the second pass is the
# "restart" case, proving the file — not process memory — is what carries the
# session across a quit.
#
# Usage: session_store_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: session_store_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
ROOT="$WORKDIR/appsupport"
mkdir -p "$ROOT"

echo "== pass 1 =="
"$BIN" "$ROOT"

echo "== pass 2 (fresh process, same root) =="
"$BIN" "$ROOT"

echo "PASS: the session round-trips, and geometry survives a close ([R-Q2])."
