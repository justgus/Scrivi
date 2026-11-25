# Closed Issue (Not Verified) — I-0375

## I-0375: `[ScriviCore]` An older build read an EMPTY file while opening a newer project — cause unknown

**Title:** An older build read an empty file while opening a newer project
**Status:** ⚪ **CLOSED — Unreproducible (2026-11-24, user-directed)**
**Reason for Closure:** the user, 2026-11-24: *"I-0323 verified as built, file the cause to unreproducable.  close it on my
authority."* Thoroughly investigated in [SP-244] and not reproduced (below).
**Platform:** `[ScriviCore]`
**Split from:** [I-0323] (2026-11-24) — I-0323 kept the MESSAGE fix (a sentence naming the file, Verified as built); this
Issue is the CAUSE it never found.
**Date Identified:** 2026-10-24, [SP-193] Mac live pass step 4: a Finder double-click launched `/Applications/Scrivi.app`
(a 2026-09-16 build, process 88249) on a current project; the open failed with *"[json.exception.parse_error.101] parse
error at line 1, column 1: attempting to parse an empty input…"* — the old build read a file that was empty or absent.
**Investigation ([SP-244], 2026-11-24):** built the 2026-09-16 code (`0a9e410`) in a scratch copy with its file reads logged:
- the old APP opened today's `the-stairs-of-tintagael` and `sp220-every-case` (14,071 and 205 file reads, none empty);
- the old CORE opened three of today's projects;
- an AppleDouble `._*.meta.json` sidecar ([I-0221]'s cause of the same message) did not fail either.
So the empty file of 2026-10-24 is not something today's format makes. The "saved by a newer version" message ([SP-244] Q3
part 2) was therefore not built.
**If it recurs:** the message now names the file on the open path (*"“project.json” is empty, so it could not be read."*,
`path` set) — open a new Issue with that file name; it is the missing evidence.
