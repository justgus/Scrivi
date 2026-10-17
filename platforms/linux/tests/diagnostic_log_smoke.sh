#!/usr/bin/env bash
# diagnostic_log_smoke.sh — SP-185 / T-0612: the diagnostic log, read back from the file. Offscreen.
#   1. the log, the watchdog's stall report, the privacy rule and the two kept sessions (the harness's own checks);
#   2. a REAL crash in its own process: the log holds "crashed", what was running and a backtrace.
#
# Usage: diagnostic_log_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: diagnostic_log_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_STATE_HOME="$WORKDIR/state"
export QT_QPA_PLATFORM=offscreen
LOG="$XDG_STATE_HOME/scrivi/scrivi.log"

echo "== Diagnostic log (file, watchdog, privacy, two sessions) =="
OUT="$("$BIN" log)" || { echo "$OUT"; exit 1; }
echo "$OUT"
echo "$OUT" | grep -qx 'diagnostic-log-ok' || { echo "FAIL: harness did not report diagnostic-log-ok"; exit 1; }

echo "== A real crash leaves its report =="
set +e
"$BIN" crash >/dev/null 2>&1
code=$?
set -e
if [ "$code" -eq 0 ]; then
    echo "FAIL: the crash run exited 0"
    exit 1
fi
fail=0
grep -q 'begin about to crash' "$LOG"                        || { echo "FAIL: the span before the crash is not logged"; fail=1; }
grep -q '\*\*\* crashed: signal 11 (SIGSEGV' "$LOG"          || { echo "FAIL: no crash line"; fail=1; }
grep -q '\*\*\* running on the main thread: about to crash' "$LOG" || { echo "FAIL: the crash does not name what was running"; fail=1; }
grep -q '\*\*\* end of crash report' "$LOG"                  || { echo "FAIL: the crash report is not complete"; fail=1; }
frames=$(sed -n '/backtrace of the crashing thread/,/end of crash report/p' "$LOG" | wc -l)
[ "$frames" -ge 5 ]                                          || { echo "FAIL: the backtrace has $frames lines"; fail=1; }
grep -q 'session ended (exit code' "$LOG"                   && { echo "FAIL: a crashed session claims it ended"; fail=1; }
grep -q 'smoke build 2' "$LOG.1"                             || { echo "FAIL: the previous session was not kept"; fail=1; }
if [ "$fail" -ne 0 ]; then
    cat "$LOG"
    exit 1
fi
echo "crash exit code $code; backtrace $((frames - 2)) frame(s)"
echo "PASS: the diagnostic log works on Linux."
