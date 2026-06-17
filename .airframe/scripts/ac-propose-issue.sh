#!/usr/bin/env bash
# Proposes a new Issue record via AICockpit.
# Usage: ac-propose-issue.sh <I-XXXX> <"title">
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 2 ]]; then
  echo "Usage: $(basename "$0") <I-XXXX> <\"title\">" >&2
  exit 1
fi

ac_run issue propose --id "$1" --title "$2"
