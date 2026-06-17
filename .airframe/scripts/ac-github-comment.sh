#!/usr/bin/env bash
# Posts a comment to the linked GitHub issue. Requires explicit approver name.
# Usage: ac-github-comment.sh <T-XXXX|I-XXXX> <"body"> <"ApproverName">
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

if [[ $# -lt 3 ]]; then
  echo "Usage: $(basename "$0") <T-XXXX|I-XXXX> <\"body\"> <\"ApproverName\">" >&2
  echo "" >&2
  echo "Approval is required. All three arguments must be provided." >&2
  exit 1
fi

ac_run github comment "$1" --body "$2" --approve --approved-by "$3" --backend github-issues
