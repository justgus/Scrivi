#!/usr/bin/env bash
# Sourced by all ac-*.sh scripts. Sets AICOCKPIT and AC_CONFIG, defines ac_run().
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
AICOCKPIT="$REPO_ROOT/../Airframe/demos/LiveDemo/bin/aicockpit"
AC_CONFIG="$REPO_ROOT/.airframe/airframe-workspace.json"

if [[ ! -x "$AICOCKPIT" ]]; then
  echo "error: aicockpit binary not found at $AICOCKPIT" >&2
  exit 1
fi

if [[ ! -f "$AC_CONFIG" ]]; then
  echo "error: airframe config not found at $AC_CONFIG" >&2
  exit 1
fi

# ac_run <subcommand...> — always passes --config
ac_run() {
  "$AICOCKPIT" "$@" --config "$AC_CONFIG"
}
