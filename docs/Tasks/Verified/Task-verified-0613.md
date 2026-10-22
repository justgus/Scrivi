# Verified Task — T-0613

**Sprint:** [SP-185] · **Epic:** [EP-026] · **Platform:** `[Linux]`
**Implemented:** 2026-10-17 · ✅ **USER-VERIFIED 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) — rig live pass step 11: a search typed in dumas over the share, no freeze or crash.
**Archived:** 2026-10-21, on the user's report of the pass.

---

## T-0613 — `[Linux]` The find bar never freezes while the writer types: search after a pause, the searchable text built ahead

**Epic:** [EP-026] · **Status:** ✅ **Verified 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) · **Sprint:** [SP-185] →
[`../../Sprints/Closed/Sprint-SP-185.md`](../../Sprints/Closed/Sprint-SP-185.md) · **Date Requested:** 2026-10-17 (user: *"I would prefer it if it
worked like search on the mac. That is, you begin typing, it allows you to type a word or a phrase, and when you stop typing
(say 500 ms), then it does all the initialization it needs to complete the search."*)

**Found:** Linux builds the searchable (presented) text of the WHOLE manuscript on the find bar's first keystroke, on the GUI
thread (`FindBar::presented()`, `FindBar.cpp:186`): 17 s under AddressSanitizer on dumas-prose, about 1.1 s in a Debug
build (`find_smoke`, 1,186 scenes), about 0.3 s in Release. Every later keystroke searches again at once.
**Apple's shape:** AppKit's `NSTextFinder` searches incrementally on a background queue, over a presented map the view keeps
live and patches per edit (`ManuscriptFind.swift:22`, `presentedMap`), so typing never waits for it.

**What:**
1. A keystroke in the query only records it; the search runs 500 ms after the last keystroke (Return, Next and Previous
   run it at once).
2. The presented text is built ahead, never on a keystroke: in small slices while the app is idle, after the load. A search
   asked for before it is complete waits for it, with the count reading "Searching…".
3. The diagnostic log ([T-0612]) times both on the rig.

**Done (2026-10-17):**
- `FindBar` (`FindBar.cpp`): a keystroke only restarts a 500 ms pause (`onQueryEdited`); Return, Next, Previous and Replace
  run a pending search at once (`flushQuery`); Replace All uses every match of what was typed. Every action passes one gate
  (`readyFor`): before the searchable text is built it is queued, the count reading "Searching…", and runs when it is.
- The searchable text is built ahead after each load, in 8 ms slices from an idle timer
  (`ManuscriptFind::PresentedText::refreshSlice`); edits still rebuild only their scene.
- Measured (Debug, Docker): 1,186 scenes built ahead over 1,132 ms, the longest slice holding the GUI thread 8 ms.
- `find_smoke` (CI): two new sections; 3 mutations caught (search per keystroke · not built ahead · the whole build at once,
  408 ms). Smokes 36/36, ctest 728/728, CI script checks pass.
