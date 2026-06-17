#!/usr/bin/env bash
# Posts a structured evidence comment to the linked GitHub issue. Requires explicit approver.
# Usage: ac-github-evidence-comment.sh <T-XXXX|I-XXXX> <EV-XXXX> <"summary"> <artifact> <"ApproverName">
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 5 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX|I-XXXX> <EV-XXXX> <\"summary\"> <artifact> <\"ApproverName\">" >&2
  echo "" >&2
  echo "Approval is required. All five arguments must be provided." >&2
  exit 1
fi

ac_run github evidence-comment "$1" \
  --id "$2" \
  --summary "$3" \
  --artifact "$4" \
  --approve \
  --approved-by "$5" \
  --backend github-issues
