#!/usr/bin/env bash
# Attaches evidence to a Task via AICockpit.
# Usage: ac-evidence.sh <T-XXXX> <EV-XXXX> <"summary"> <artifact-path>
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 4 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX> <EV-XXXX> <\"summary\"> <artifact-path>" >&2
  exit 1
fi

ac_run evidence attach "$1" --id "$2" --summary "$3" --artifact "$4"
