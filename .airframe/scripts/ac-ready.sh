#!/usr/bin/env bash
# Marks a Task as Implemented - Not Verified (ready for human review).
# Usage: ac-ready.sh <T-XXXX>
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 1 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX>" >&2
  exit 1
fi

ac_run work ready "$1"
