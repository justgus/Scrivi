# Closed Issue (Not Verified) — I-0201

## I-0201: `[Apple]` A launch argument silently prevents the app from starting

**Status:** ⚪ **CLOSED — Overtaken by Events (2026-10-03, user-approved)**
**Reason for Closure:** ⚠️ **OBE — the launch arguments were part of an approach the project no longer uses.**
**Platform:** macOS (`[Apple]` only)
**Component:** `Info.plist` (`CFBundleDocumentTypes`) · Xcode scheme arguments
**Severity at closure:** High (as filed)
**Sprint at closure:** none — found during [SP-132], never assigned
**Date Identified:** 2026-09-12 · **Date Closed:** 2026-10-03

---

### ✅ THE RULING (user, 2026-10-03)

⚠️ ***"this is no longer an issue [Apple only]. we were trying to launch differently and so this issue is OBE."***

✅ **Closed as no longer applicable, ⛔ not as work completed.** The behaviour itself is unchanged: AppKit
still treats a non-option launch argument as a document to open, because the app declares
`CFBundleDocumentTypes`. It only mattered while launch arguments were being used, and they no longer are.

✅ **What stays in the code:** the warnings added in `a50ebc9` (2026-09-12) remain at
`Scrivi/App/ScriviApp.swift:12` and `Scrivi/App/AppEnvironment.swift:319`, `:364`. Use an
environment variable for any launch-time switch (`SCRIVI_DIAG_TIMING` already does).

⚠️ **Found at closure (2026-10-03):** the row's status still read *"the constraint is undocumented in the
codebase"*, but those warnings had been in the code since the day the Issue was filed.

---

### Original record (verbatim from `Issue-active.md`)

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0201** | `[Apple]` ⚠️ **A LAUNCH ARGUMENT SILENTLY PREVENTS THE APP FROM STARTING — no window, no console, no crash, NO THREADS.** ⚠️ **Found 2026-09-12 by the USER, after a full bisect cleared every line of code.** ⚠️ **Passing ANY unrecognised argument in the Xcode scheme (Run ▸ Arguments ▸ Arguments Passed On Launch) makes ⌘R produce NOTHING**: ✅ **the Debug Navigator shows CPU idle, memory flat at `23.8 MB`, disk and network `0`, and ⚠️ NO VISIBLE THREADS** — ⚠️ **which is the tell: a running app always has threads, so the process never really starts.** ---- ✅ **ROOT CAUSE: Scrivi is a DOCUMENT-BASED APP.** ⚠️ **`Info.plist` declares `CFBundleDocumentTypes`** (`com.caposoft.scrivi.project` as Owner, `com.caposoft.scrivi.world` as Alternate) — ✅ **so AppKit treats non-option launch arguments as DOCUMENTS TO OPEN**, ⚠️ **resolves the argument as a file path, finds nothing, and abandons the launch BEFORE the app initialises.** ⚠️ **THE APP PARSES NO ARGUMENTS AT ALL** — ✅ **`grep` for `CommandLine.arguments` in `Scrivi/` returns NOTHING** — ⚠️ **so this is NOT the app rejecting the flag; it never runs.** ---- ✅ **ENVIRONMENT VARIABLES ARE UNAFFECTED and are the correct mechanism**: ⚠️ **they never reach the document-launch path.** ✅ **`SCRIVI_DIAG_TIMING` has worked in that same scheme throughout.** ---- ⚠️ **WHY THIS COST AN AFTERNOON:** ⚠️ **the failure is COMPLETELY SILENT and looks exactly like an app-code hang**, ✅ **so a bisect of the working tree cleared docs, ScriviCore AND Linux one by one and found nothing** — ⚠️ **because the culprit was the ONE VARIABLE THE BISECT NEVER ISOLATED: the scheme itself.** ⚠️ **I twice attributed it to my own code and once to a stale `libScriviCore.a`; ✅ ALL THREE WERE WRONG, and the user found it by removing the scheme argument.** ---- ⚠️ **THIS IS NOT ONLY A TEST-RIG PROBLEM.** ⚠️ **It means NO launch argument can be given to this app from Xcode, by anyone, ever** — ✅ **and nothing anywhere says so.** ⚠️ **A future contributor adding a debug flag will lose the same afternoon.** ✅ **FIX DIRECTION: (a) DOCUMENT the constraint where a flag would be added; (b) prefer ENV VARS for every launch-time switch; ⚠️ (c) if an argument is ever genuinely required, it must be consumed before AppKit's document handling, which is a bigger change than it looks.** | ⚠️ **HIGH** | ⚠️ **Unassigned** — ⚠️ **found during [SP-132]** | 🔵 **Open — ✅ ROOT CAUSE ESTABLISHED BY EXPERIMENT** (add the argument ⇒ no window; remove it ⇒ clean launch, reproduced by the user both ways). ⚠️ **NOT yet fixed: the constraint is undocumented in the codebase.** |
