#!/usr/bin/env bash
# Transitions a GitHub issue status label. Requires explicit approver name.
# Usage: ac-github-status.sh <T-XXXX|I-XXXX> <active|unverified|backlog|closed> <"ApproverName">
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 3 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX|I-XXXX> <active|unverified|backlog|closed> <\"ApproverName\">" >&2
  echo "" >&2
  echo "Approval is required. All three arguments must be provided." >&2
  exit 1
fi

WORK_ID="$1"
STATUS="$2"
APPROVER="$3"

case "$STATUS" in
  active|unverified|backlog|closed) ;;
  *)
    echo "error: invalid status '$STATUS'. Must be one of: active, unverified, backlog, closed" >&2
    exit 1
    ;;
esac

ac_run github status "$WORK_ID" --to "$STATUS" --approve --approved-by "$APPROVER" --backend github-issues
