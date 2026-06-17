#!/usr/bin/env bash
# Proposes a new Task record via AICockpit.
# Usage: ac-propose-task.sh <T-XXXX> <"title">
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 2 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX> <\"title\">" >&2
  exit 1
fi

ac_run task propose --id "$1" --title "$2"
