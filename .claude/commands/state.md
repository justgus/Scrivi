---
description: Print the real state of tracked work, read from the files
allowed-tools: Bash(./scripts/work-state.sh)
---

!`./scripts/work-state.sh`

⚠️ **Audit ruling [R-17], 2026-09-15.** The output above is read from the tracking files, not from
memory or from a status line. Status lines go stale — [T-0529]'s was wrong for a day, [T-0521]'s
claimed code that was never written.

Do not re-narrate the output. State what was done, normally.
