#!/usr/bin/env bash
# Fetches the full task packet for a given Task ID.
# Usage: ac-packet.sh <T-XXXX>
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 1 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX>" >&2
  exit 1
fi

ac_run task packet "$1" --backend github-issues --output json
