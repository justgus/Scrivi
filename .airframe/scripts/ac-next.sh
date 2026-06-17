#!/usr/bin/env bash
# Returns the next active task, or reports no active task.
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

ac_run task next --backend github-issues --output json
