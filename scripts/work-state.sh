#!/usr/bin/env bash
# Print the REAL state of tracked work, read from the files — never from memory.
# Written 2026-09-15 after the user asked, correctly: "How can I understand what is
# done and what is not done when you consistently fail to provide an overview?"
#
# ⚠️ Status LINES go stale. This reads every layer and shows DISAGREEMENTS, which is
# what hid T-0529 (stale for a day) and T-0521 (row said "Implemented"; code untouched).
cd "$(dirname "$0")/.." || exit 1

echo "═══ EPICS ═══"
grep -hoE "^\| EP-0[0-9]{2} \|[^|]*\|[^|]*" docs/Epics/Epic-Documentation.md 2>/dev/null \
  | grep -E "ACTIVE|Draft|Complete" | sed 's/|/ /g' | sed 's/  */ /g'

echo
echo "═══ SPRINTS ═══"
a=$(grep -c "^## SP-1" docs/Sprints/Sprint-active.md 2>/dev/null)
echo "  active:  $a"
echo "  closed:  $(ls docs/Sprints/Closed/ 2>/dev/null | wc -l | tr -d ' ')"
echo "  backlog: $(grep -c "^## \(⚠️ \)\?SP-1" docs/Sprints/Sprint-backlog.md 2>/dev/null)"

echo
echo "═══ TASKS BY LAYER ═══"
printf "  active:     %s\n" "$(grep -c '^| \*\*T-0' docs/Tasks/Task-active.md 2>/dev/null)"
printf "  unverified: %s\n" "$(grep -c '^| \*\*T-0' docs/Tasks/Task-unverified.md 2>/dev/null)"
printf "  backlog:    %s\n" "$(grep -c '^| \*\*T-0' docs/Tasks/Task-backlog.md 2>/dev/null)"
printf "  verified:   %s files\n" "$(ls docs/Tasks/Verified/ 2>/dev/null | wc -l | tr -d ' ')"

echo
echo "═══ ⚠️ TASKS CLAIMING 'Implemented' BUT NEVER VERIFIED ═══"
# ⚠️ A claim is only OPEN if the ID is not ALSO archived under Verified/.
# 2026-09-15: the earlier version omitted this subtraction and reported T-0419 as
# missing when it was archived all along (Verified/ is a SUBDIRECTORY — docs/Tasks/*.md
# never reached it). Report a gap only after looking where the answer would live.
claimed=$(grep -h "Implemented - Not Verified" docs/Tasks/*.md docs/Sprints/Sprint-active.md 2>/dev/null \
  | grep -oE "T-0[0-9]{3}" | sort -u)
archived=$(grep -rhoE "T-0[0-9]{3}" docs/Tasks/Verified/ 2>/dev/null | sort -u)
open_claims=$(comm -23 <(echo "$claimed") <(echo "$archived") | tr '\n' ' ')
if [ -n "${open_claims// /}" ]; then
  echo "$open_claims"
  echo "  ⚠️ VERIFY THESE AGAINST THE CODE, NOT THE ROW."
else
  echo "  ✅ none — every 'Implemented' claim is archived under Verified/."
fi

echo
echo "═══ ⚠️ OPEN ISSUES ═══"
grep -oE "^\| \*\*I-0[0-9]{3}\*\*" docs/Issues/Issue-active.md 2>/dev/null \
  | grep -oE "I-0[0-9]{3}" | tr '\n' ' '; echo

echo
echo "═══ ⛔ ORPHANS: tasks in a Sprint file but NO task layer ═══"
for t in $(grep -hoE "T-0[0-9]{3}" docs/Sprints/Sprint-active.md docs/Sprints/Closed/*.md 2>/dev/null | sort -u); do
  n=$(grep -l "$t" docs/Tasks/Task-active.md docs/Tasks/Task-unverified.md \
        docs/Tasks/Task-backlog.md docs/Tasks/Verified/*.md 2>/dev/null | wc -l | tr -d ' ')
  [ "$n" = "0" ] && printf "%s " "$t"
done
echo
