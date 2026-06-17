#!/usr/bin/env bash
# Idempotently creates required Airframe labels in the GitHub repo.
# Usage: ac-github-bootstrap-labels.sh [--repo owner/repo]
set -euo pipefail
source "$(dirname "$0")/_ac_common.sh"

# Default repo from config
REPO="justgus/Telemetrix"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --repo) REPO="$2"; shift 2 ;;
    *) echo "Usage: $(basename "$0") [--repo owner/repo]" >&2; exit 1 ;;
  esac
done

echo "Bootstrapping Airframe labels in $REPO ..."
echo ""

# Required labels: name, color, description
declare -a LABELS=(
  "airframe-task|0075ca|Airframe planned work item (Task)"
  "airframe-issue|d73a4a|Airframe bug or unintended behavior (Issue)"
  "status-closed|cfd3d7|Work item closed"
  "sprint-SP-001|bfd4f2|Belongs to Sprint SP-001"
)

# Fetch existing label names once
EXISTING="$(gh label list --repo "$REPO" --limit 100 --json name --jq '.[].name')"

for entry in "${LABELS[@]}"; do
  IFS='|' read -r name color description <<< "$entry"
  if echo "$EXISTING" | grep -qx "$name"; then
    echo "  [skip]   $name (already exists)"
  else
    gh label create "$name" --repo "$REPO" --color "$color" --description "$description"
    echo "  [create] $name"
  fi
done

echo ""
echo "Done. All required Airframe labels are present in $REPO."
echo ""
echo "--- Minimum GitHub Issue body template ---"
cat <<'EOF'
Airframe Type: Task
Airframe ID: T-XXXX
Epic: EP-XXX
Sprint: SP-XXX
Priority: High|Medium|Low|Critical
Status: Backlog|Active|Implemented - Not Verified|Implemented - Verified|Closed
EOF
