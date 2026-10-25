---
sprint: SP-193
epic: EP-054
status: Active
platform: Cross
created: 2026-10-24
---

# SP-193 — `[Cross]` [EP-054] **S0**: fix before the alpha, and one version on every platform

**Status:** 🟡 **ACTIVE** 2026-10-24 — ✅ AC1–AC5 met; ⏳ AC6 live passes — (user: *"keep them in the backlog. I don't think they pose a major threat to data. Then activate SP-193"*) — created 2026-10-24 (user: *"file those three as fix-before-alpha.
Let's put it in a sprint in EP-054."*). ⏳ Q1–Q2 to rule.
**Carries:** [I-0306], [I-0313], [I-0202] (→ [`../Issues/Issue-active.md`](../Issues/Issue-active.md)) · [T-0622]
(✅ approved 2026-10-24; 🟢 active → [`../Tasks/Task-active.md`](../Tasks/Task-active.md)).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) (🟡 Active with this Sprint). ⚠️ This Sprint is EP-054's S0: the
alpha's prerequisites, ahead of S1 (the archive).
**Size:** S–M.

---

## Goal

Before any tester writes real work in Scrivi, the three open Issues that can lose or misfile text, or crash at launch,
are fixed. Every platform also reports one version, `1.0.<commits on main>`, with the commit's hash beside it.

## The items (read 2026-10-24; re-read at activation)

| Item | What | Where |
| ---- | ---- | ----- |
| [I-0306] `[Apple]` | File ▸ Close Project (⌘W) calls `NSWindow.close()`, which never asks `windowShouldClose`, so the "Some changes can't be saved" question ([T-0570]) is skipped. With the project's drive pulled, those edits are lost. | `ProjectWindowManager.swift` (`closeWindow` → `close()`; `windowShouldClose`) |
| [I-0313] `[Apple]` | Typing into an empty scene that sits right before another empty scene is credited to the NEXT scene: the page shows it in the first, but the boundary table grows the second and the scene list (so the save) gives it to the second. Reproduced in the interop harness, not yet in the app. | `ManuscriptTextView` (`textDidChange` → `segmentIndex(for:)`), `SceneBoundaryTable.apply` |
| [I-0202] `[Apple]` | `AppDelegate.application(_:open:)` and `applicationWillTerminate` wrap their bodies in `MainActor.assumeIsolated`, which TRAPS if not on the main actor. `application(_:open:)` is the Finder double-click path and can arrive during launch. Found by reading; never seen to crash. | `ScriviApp.swift:126–145` |
| [T-0622] `[Cross]` | Version `1.0.<git rev-list --count HEAD>` (Apple's build number = the count), the short hash (and `-dirty`) shown in Help ▸ About and `scrivi_linux --version`, all read from git at build time; the Linux counter (`bump-build-stamp.sh`, build 79) retired. | Xcode build phase → built Info.plist; Linux CMake → the stamp header; `AboutView.swift`; Linux About and `--version` |

## Plan (draft)

0. **Red tests first:** ⌘W with unsavable changes asks; typing into the first of two empty scenes stays in the first
   (the scene list and the saved file); the version string matches git's count and hash.
1. **I-0306:** route ⌘W through the same check as the red button (`performClose`, or the shared confirm), after reading
   every caller of `close()` (the Issue's own caution).
2. **I-0313:** instrument first (the Issue: *"prove it by instrument before fixing"*), then fix the attribution at its
   root: the boundary table or `segmentIndex(for:)`, whichever the trace shows. ⚠️ Linux: check whether its editor has
   the same shape (`feedback_linux_adopts_apple_shape`).
3. **I-0202:** first establish whether it can trap at all (Q1); then fix or close accordingly.
4. **T-0622:** the build-time stamp on both platforms, About and `--version`, the counter retired, `CLAUDE.md`'s rig
   notes and the rig setup doc updated.
5. **Tests**, mutation-checked in a copy of the repo; ctest, interop, Linux smokes (Docker).
6. **Live passes:** Mac (⌘W with the drive pulled; two empty scenes; About), and the rig (About / `--version` from a
   pulled commit).

## Questions — ✅ ruled 2026-10-24 (user: *"confirmed for both."*)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **I-0202 may not be reachable:** in the macOS 26/27 SDK `NSApplicationDelegate` is a `@MainActor` protocol, so its methods are already main-actor isolated and `assumeIsolated` would only re-check what is true. | **Check the SDK's annotation (Apple's docs JSON) first.** If the delegate is `@MainActor`, remove the redundant `assumeIsolated` and close I-0202 as not reachable; otherwise `Task { @MainActor in … }`, keeping `applicationWillTerminate` synchronous (the Issue's caution). |
| **Q2** | **T-0622 on a dirty tree:** a Debug build from uncommitted changes shows `-dirty`. A TestFlight upload or a `.deb` from a dirty tree — allowed? | **Refuse** a TestFlight archive or a `.deb` built from a dirty tree, so every tester's build names a real commit. |

## Acceptance Criteria (draft)

- [x] AC1 — [I-0306]: ⌘W with unsavable changes asks the same question as the red button; tested through `close()`
- [x] AC2 — [I-0313]: typing into the first of two empty scenes stays in that scene: on the page, in the scene list and
      on disk; the cause shown by instrument first
- [x] AC3 — [I-0202]: resolved per Q1 (fixed, or closed as unreachable with the SDK's evidence)
- [x] AC4 — [T-0622]: Mac and Linux show `1.0.<count> (<hash>)`, the same for the same commit; Apple's build number is
      the count; the Linux counter is gone
- [x] AC5 — Tests, mutation-checked; ctest, interop and Linux smokes green
- [ ] AC6 — Live passes (user): Mac and rig

## Progress log

### 🔵 2026-10-24 — CREATED in Planning
- User: *"T-0622 approved; file those three as fix-before-alpha. Let's put it in a sprint in EP-054."* Issued via
  `next-id.py`. T-0622 included as the alpha's other prerequisite (noted, not ruled; it can move out).
- [I-0312] closed OBE the same day (user). ⚠️ [I-0223] and [I-0192] were NOT closed: they are not legacy-scheme issues
  (a world copy or older version; a world on an unplugged drive), and [EP-054]'s archive will create I-0223's case.
  Awaiting the user's call.

### 🟡 2026-10-24 — ACTIVATED
- User: *"keep them in the backlog. I don't think they pose a major threat to data. Then activate SP-193"* [I-0223] and [I-0192] → the Issue backlog ([EP-044]). [T-0622] → active. [EP-054] → 🟡 Active
  (its first Sprint).

### 🟡 2026-10-24 — Q1 evidence (read from the SDK)
- The installed SDK (`MacOSX27.2.sdk`, `AppKit.framework/Headers/NSApplication.h`) marks both delegate methods
  `NS_SWIFT_UI_ACTOR`, Swift's `@MainActor`: `application:openURLs:` (`:417`) and `applicationWillTerminate:` (`:529`).
  `AppDelegate` declares `NSApplicationDelegate` on its own declaration (`ScriviApp.swift:26`), so its witnesses are
  main-actor isolated, and AppKit calls them on the main thread. The two `MainActor.assumeIsolated` wrappers
  (`ScriviApp.swift:127`, `:143`) re-assert what the compiler already guarantees: they cannot trap.
- ⏳ For the user: Q1 as recommended (remove the two wrappers, which proves the isolation by compiling, then close
  [I-0202] as not reachable), and Q2.

### 🟡 2026-10-24 — Q1–Q2 confirmed; IMPLEMENTED (AC1–AC5 met); ⏳ AC6 live passes
- User: *"confirmed for both."*
- **[I-0202]** (AC3): the two `assumeIsolated` wrappers removed; it compiles in Swift 6 with no warnings, which proves
  the isolation. → ⚪ CLOSED, not reachable (`Issues/Closed/Issue-closed-0202.md`).
- **[I-0306]** (AC1): `close()` asks `windowShouldClose` itself (not `performClose`, which beeps on a refusal). Interop
  `closeProjectAsksAboutUnsavable`; mutation caught. → 🟠 Resolved - Not Verified. ⚠️ **Found:** Linux has NO
  unsavable-changes question at all ([T-0570] was Apple-only). Not this Sprint's; ⏳ for the user (file it?).
- **[I-0313]** (AC2): the probe showed the storage's edit was exact; the typed text carried `scrivi.divider` (AppKit's
  typing attributes at a line start come from the character after the caret: the divider). The coordinator's
  `shouldChangeTypingAttributes` gives typing the body's attributes then. Interop `typingIntoFirstOfTwoEmptyScenes`;
  mutation caught. Linux: a smoke block shows it never had it. → 🟠 Resolved - Not Verified.
- **[T-0622]** (AC4): see the Task record; Mac, iOS and Linux checked. → 🟠 Implemented - Not Verified.
- **Counts:** ctest 766/766; interop 316/316, no warnings; Linux (Docker, Qt 6.4) builds; smokes 38/38.

### 🟡 2026-10-24 — Mac live pass (user): steps 1–2 ✅, 3 ❌ → fixed, 4 ❌ (an OLD build), 5 ⏳ (rig off)
- ✅ Step 1 (⌘W with the drive pulled asks) and ✅ step 2 (typing into the first of two empty scenes).
- ❌ **Step 3, About:** *"No dialog box is shown."* Not [T-0622]'s stamp: About was a SwiftUI sheet on the WELCOME
  window, which is not on screen while a project is open, so it had nowhere to appear (an older defect, surfaced
  here). ✅ Fixed: macOS About is AppKit's standard panel (`AboutPanel.show()`, `AboutView.swift`), reading
  "Version 1.0.<count> (<commit>)" (`ApplicationVersion` + `Version`, NSApplication.h); the Welcome sheet removed;
  iOS keeps its sheet. Interop `aboutPanelShowsTheVersion` opens it with a project window up and reads the version
  back from the panel. Interop 317/317, no warnings. ⏳ Re-check.
- ❌ **Step 4, Finder double-click:** *"[json.exception.parse_error.101] … empty input"*. ⚠️ The unified log shows the
  launched process (88249, 17:40:40) was **`/Applications/Scrivi.app`, a build from 2026-09-16**: LaunchServices had 13
  registered copies of Scrivi.app, all version "1.0". Not today's code. ✅ My four scratch builds unregistered and
  deleted. ⏳ The user's `/Applications/Scrivi.app` and `~/Desktop/Scrivi-Probe-Build/Scrivi.app` are the user's call.
  ⚠️ Finding: an OLDER build opening a newer project fails with a raw parser error, not a plain message.
- ⏳ **Step 5:** the rig is powered off (`rig-build.sh`: cannot reach oathkeeper after 30 s).

### 🟡 2026-10-25 — two findings filed (user: *"yes, file both Issues"*)
- [I-0322] `[Linux]` no project-drive-lost handling ([T-0570]'s Linux half), and [I-0323] `[ScriviCore]` the raw parser
  error from an older build: both → the Issue backlog, unassigned. Neither is this Sprint's.

### 🟡 2026-10-25 — Mac re-checks ✅ (user: *"I did the rechecks. they both pass."*)
- ✅ Step 3 (About shows `Version 1.0.<n> (<hash>)` with a project open) and ✅ step 4 (Finder double-click opens the
  project, with the old copies removed).
- [I-0306] and [I-0313] → ✅ **Verified** (steps 1–2) and archived. [T-0622]: the Mac half passed; ⏳ it stays
  Implemented - Not Verified until the rig pass (step 5), which is all AC6 now waits on.
