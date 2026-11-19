---
sprint: SP-224
epic: EP-055
status: Closed
closed: 2026-11-19
platform: Apple
created: 2026-11-19
---

# SP-224 — `[Apple]` [EP-055] S4: projects and worlds survive a relaunch on iPad and iPhone

**Status:** ✅ **CLOSED** 2026-11-19 (user, 2026-11-19: *"all steps pass, close SP-224"*). Activated 2026-11-19 (user, 2026-11-19: *"Q1-Q3 as recommended, … activate and implement SP-224"*) · planned 2026-11-19 (user: *"go, plan SP-224"*) · ✅ Q1–Q3 ruled as recommended 2026-11-19.
**Carries:** [T-0687], [I-0364], [I-0365] — ✅ Verified 2026-11-19 → [`../../Tasks/Verified/Task-verified-0687.md`](../../Tasks/Verified/Task-verified-0687.md), [`../../Issues/Verified/Issue-verified-0361-0370.md`](../../Issues/Verified/Issue-verified-0361-0370.md).
**Epic:** [EP-055] → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md) — AC7 (Q2 ruled 2026-11-19: Files / iCloud Drive, external
drives on iPad, the archive).
**Builds on:** EP-017 T-0184 (`ProjectBookmarkStore`), SP-104 (`WorldBookmarkStore`), R4 / T-0195 (`OpenSessionManifest`,
`restoreOpenProjects`).
**Size:** S — the plumbing exists on every platform; only the two stores are Mac-only.

---

## What reading the code found (2026-11-19)

- **Every iOS open path already records a bookmark.** `loadProject(at:bookmarkURL:)` calls `ProjectBookmarkStore.record`
  (`AppEnvironment.swift:469`) for a project picked in Files (`:744`, from `LandingView.swift:106`), a new project (`:691`, `:721`) and an
  imported one (`:844`). The manifest is saved on every open and close (`persistOpenManifest`, `:255`), so it is current when iOS kills
  the app in the background. At launch `restoreOpenProjects` (`:305`) reopens each saved project through `ensureOpenAndShow` (`:634`).
- ⚠️ **On iOS both stores do nothing.** `ProjectBookmarkStore.record` and `resolve` are `#if os(macOS)`; `resolve` returns nil
  (`ProjectBookmarkStore.swift:70`). `WorldBookmarkStore.record` stores nothing and `activate` returns false
  (`WorldBookmarkStore.swift:38`, `:98`). So `hasBookmark` is false at launch, restore skips every project, and the writer lands on the
  Landing view and must go into Files again. A world that came with an imported archive (`ProjectArchiveFlow.swift:670`) is unreadable
  after a relaunch: `missing`.
- **The reason is one option.** `.withSecurityScope` exists only on macOS. Apple's iOS shape (*"Providing access to directories"*,
  UIKit, fetched 2026-11-19): record with `bookmarkData(options: .minimalBookmark, …)`, resolve with
  `URL(resolvingBookmarkData:bookmarkDataIsStale:)`, then `startAccessingSecurityScopedResource()`. The rest of each store (map, stale
  refresh, held scopes) is already platform-neutral.
- ⚠️ **A project picked in Files never releases its scope.** `openProjectFromPickedURL` keeps the access it started for the session
  (`AppEnvironment.swift:741`, its note says *"a future iOS-close path"*). The restored path already releases on close
  (`deepLinkAccessURL`, `ProjectSession.swift:447`).
- On a failed restore (drive unplugged, permission revoked in Settings ▸ Privacy ▸ Files and Folders), `ensureOpenAndShow` reports
  *"This project could not be reopened. Open it once from the Welcome window."* (`:642`). iOS has no Welcome window.
- No Recent Projects list exists on either platform. Files' own Recents tab serves on iOS. ⛔ Not added.

## Goal

A tester opens a project on an iPad or iPhone from **Files, iCloud Drive or (iPad) an external drive**, quits the app or lets iOS end it,
and the next launch **reopens the project**, with any shared world readable. Nothing changes on the Mac.

## Plan

1. **The two stores (Q1):** `#if os(macOS)` keeps only the options (`.withSecurityScope` on the Mac, `.minimalBookmark` / none on
   iOS); record, resolve, stale refresh and activation run on both.
2. **The picked project's scope (Q2):** `openProjectFromPickedURL` hands its started scope to `session.deepLinkAccessURL`, as the
   restored path does, so closing the project releases it.
3. **The failed-restore message (Q3).**
4. **Checks:** iOS tests on the simulator: a project bookmark round trip (record → resolve → readable, scope started) and a world
   bookmark round trip (record → activate → true); Mac interop green (`scripts/run-interop-tests.sh`, N > 0); macOS and iOS builds; CI.
5. **Live passes (user):** iPad and iPhone, below; the Mac re-checked (relaunch restores a project and a world).

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **The iOS bookmark shape.** | **Apple's:** `.minimalBookmark` to record, no options to resolve, then start access. The Mac keeps `.withSecurityScope`. One store per kind, as now; only the options differ. | ✅ As recommended (2026-11-19) |
| **Q2** | **Release a Files-picked project's scope on close?** | **Yes**, through `deepLinkAccessURL` (one line). Every reopen otherwise starts another scope that is never stopped, and the system limits how many an app may hold. | ✅ As recommended (2026-11-19) |
| **Q3** | **A project that cannot be restored** (drive unplugged, iCloud Drive signed out, permission revoked). | **As the Mac:** skip it and land on the Landing view with a message, here *"This project could not be reopened. Open it again from Files."* (iOS); the Mac's wording unchanged. Its bookmark is kept, so the next open from Files works and refreshes it. | ✅ As recommended (2026-11-19) |

⛔ **NOT in this Sprint:** world Create / Add Existing / Relink on iOS (S5, AC6); a Recent Projects list. ⛔ **iCloud Drive files not
yet downloaded are out of scope for Scrivi** (user, 2026-11-19: *"I've already opened projects on both the iPad and iPhone that hadn't
been downloaded by iCloud yet.  This is out of scope for Scrivi.  The file open dialog handles that internally."*).

## Acceptance Criteria

- [x] AC1 — ✅ built 2026-11-19 (⏳ AC5) — On iOS a project opened from Files, iCloud Drive or (iPad) an external drive is reopened at the next launch (Q1)
- [x] AC2 — ✅ built 2026-11-19 (⏳ AC5) — On iOS a world that came with an imported archive is readable after a relaunch (`available`, not `missing`)
- [x] AC3 — ✅ built 2026-11-19 (⏳ AC5) — Closing a Files-picked project releases its scope (Q2); a project that cannot be restored lands on the Landing view with Q3's message
- [x] AC4 — ✅ tests and builds 2026-11-19 (⏳ CI runs on the user's push) — iOS tests for both round trips on the simulator; Mac interop green; macOS and iOS builds; CI green
- [x] AC5 — ✅ 2026-11-19 — **Live passes (user):** iPad (iCloud Drive), iPhone (iCloud Drive), and the Mac re-checked; the app KILLED (Xcode, or the device) before each relaunch. ⛔ On-device storage and an external drive: out of scope (user, 2026-11-19). ✅ iPad steps 2, 5, 6 (2026-11-19)
- [x] AC6 — ✅ 2026-11-19 — [I-0364] a project opens from the Files app (tap, Open With); [I-0365] the Landing's buttons fit a compact width

## Progress log

- **2026-11-19** — 🔵 **Created in Planning** (user, 2026-11-19: *"go, plan SP-224"*), issued via `next-id.py`; [T-0687] filed.
  Q1–Q3 to rule.
- **2026-11-19** — ✅ Q1–Q3 ruled as recommended; iCloud Drive downloads ruled out of scope (user). 🟡 **Activated** (user, 2026-11-19: *"Q1-Q3 as recommended, … activate and implement SP-224"*); [T-0687] →
  active.
- **2026-11-19** — **Built.**
  - **Q1:** both stores now record and resolve on iOS. The `#if os(macOS)` is narrowed to the options
    (`ProjectBookmarkStore.recordOptions` / `resolveOptions`: `.withSecurityScope` on the Mac, `.minimalBookmark` / none on iOS),
    which `WorldBookmarkStore` uses too (`ProjectBookmarkStore.swift:21–30`). Record, resolve, the stale refresh and world
    activation are unchanged and now run on both.
  - **Q2:** `openProjectFromPickedURL` hands its scope to `session.deepLinkAccessURL`, so closing the project releases it; when the
    project was already open (R3) the session keeps the scope it holds and the new one is released at once
    (`AppEnvironment.swift`, `openProjectFromPickedURL`).
  - **Q3:** `AppEnvironment.cannotReopenMessage`: iOS *"This project could not be reopened. Open it again from Files."*; the Mac's
    wording unchanged.
  - **AC4 so far:** iOS 33 passed, 0 failed (iPhone 18 Pro 27.2, by ID): 2 new (`IOSBookmarkTests`: a project bookmark records and
    resolves to its folder; a world bookmark records and is forgotten). Both fail on the old code, whose iOS `record` stored nothing.
    Mac interop 352 passed, 0 failed (`scripts/run-interop-tests.sh`); macOS build ✅, no warnings in the touched files. ⏳ CI after
    the push.
- **2026-11-19** — 🟠 **iPad live pass (user):** *"when I opena project on the iCloudDrive it opens just fine, and reopens when entering
  the app again … steps 1, 3, and 4 are out of scope.  2, 5, and 6 pass."* ✅ Steps 2, 5, 6. ⛔ Steps 1, 3, 4 out of scope: the iPad has
  no picker of its own (file access is the Files app or the sandbox), and no external drive (a corner case for Files). ⚠️ The user:
  swiping Scrivi away, or ⌘Q on the iPad, only backgrounds it; **a relaunch test must kill the app** (Xcode, or the device). Two
  findings, fixed:
  - **[I-0364]** Files lists no Scrivi for a project (tap, Open With). Cause: `Info-iOS.plist` declared only the archive. ✅ The Mac's
    three document types and its project/world declarations, open in place; iOS's `openProjectFile` closes the open project and goes
    through `openProjectFromPickedURL`. Checked in the built app's merged Info.plist.
  - **[I-0365]** The Landing's buttons clipped on the folded Duo. ✅ `ViewThatFits` row/column; `minWidth: 400` on the Mac only.
  - ✅ iOS 33 passed, 0 failed; Mac interop 352 passed, 0 failed; macOS build ✅. ⏳ The re-pass (iPhone, Files, the Duo, the Mac).
- **2026-11-19** — ⛔ **Re-pass (user):** Scrivi opens from a Files tap, but *"it also tries to load the last project opened … before or after that completes, the dumas project is loaded … This time the sentinel of centauri was loaded by default"*. **Cause:** a cold launch ran `.onOpenURL`'s open and the
  session restore together; iOS shows one project but registered both, and the last load to finish took the screen; the hidden one stayed
  in the manifest (the second symptom). ✅ **Fixed (iOS only):** `.onOpenURL` records `launchURL` before awaiting; the restore skips when
  it is set, closes a restored load that finishes after a tap unless it is the tapped project (`AppEnvironment.samePackage`: no trailing
  slash, no `/private` symlink), and restores at most one project. iOS 34 passed, 0 failed (1 new: `samePackage`); macOS build ✅; Mac
  interop 352 passed, 0 failed. ⚠️ The restore is suppressed under tests (I-0150), so the race itself is checked by the re-pass only.
- **2026-11-19** — ✅ **Re-pass (user): all steps pass** (iPad, iPhone, the folded Duo, the Mac). AC5, AC6 met; [T-0687], [I-0364],
  [I-0365] → ✅ Verified and archived. ✅ **CLOSED** (user, 2026-11-19: *"all steps pass, close SP-224"*).

*Last Updated: 2026-11-19 (**✅ CLOSED** (user, 2026-11-19: *"all steps pass, close SP-224"*).)*

*Last Updated: 2026-11-19 (I-0364 re-pass: the launch restore raced the Files tap; fixed.)*

*Last Updated: 2026-11-19 (iPad steps 2, 5, 6 ✅; I-0364, I-0365 found and fixed; AC6 added.)*

*Last Updated: 2026-11-19 (**built** — AC1–AC3 met; AC4 awaits CI; T-0687 → 🟠.)*

*Last Updated: 2026-11-19 (🟡 activated; Q1–Q3 ruled as recommended.)*

*Last Updated: 2026-11-19 (created in 🔵 Planning; Q1–Q3 to rule.)*
