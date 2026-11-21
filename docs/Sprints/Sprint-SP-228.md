---
sprint: SP-228
epic: EP-055
status: Planning
platform: Apple
created: 2026-11-20
---

# SP-228 — `[Apple]` [EP-055] S8: TestFlight for iPad, iPhone and the Mac

**Status:** 🔵 **Planning** (user, 2026-11-20: *"Let's to AC9 on EP-055"*). ✅ Q1–Q3, Q5 ruled as recommended; ⏸ Q4 and the Sprint wait on [T-0684] (the rename), which the user takes first (2026-11-20).
**Carries:** [T-0690].
**Epic:** [EP-055] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — **AC9**, the last open AC: TestFlight builds for iPad/iPhone
**and the Mac** (Q9), installable by external testers. Q5 (2026-10-24): *"yes, I am all setup for testflight."* Builds are Debug.
**Builds on:** [SP-193] / [T-0622] (one version everywhere: `scripts/stamp-version.sh`).
**Size:** S–M — mostly configuration; the size depends on what the first archive refuses.

---

## What reading the project found (2026-11-20)

**Already in place:**
- ✅ **One bundle ID on both platforms,** `com.caposoft.scrivi`, team `QEJ2Z74RY9`, automatic signing. One App Store Connect record
  can carry iOS and macOS builds.
- ✅ **The version is stamped at build time** (`scripts/stamp-version.sh`, a Run Script phase on both app targets): version
  `<VERSION>.<commits>` (today 1.0.413), build `<commits>` (which TestFlight needs to increase), `ScriviCommit` = the short hash.
  ⚠️ **An archive from a tree with uncommitted changes is refused** ([SP-193] Q2), so every archive names a commit.
- ✅ **The archive action is Debug** in both schemes (`ScriviApp.xcscheme:92`, `ScriviApp-iOS.xcscheme:84`), as ruled (the alpha
  is Debug).
- ✅ **App icons are complete:** iOS 1024 pt light, dark and tinted; the Mac's ten sizes.
- ✅ The Mac app is sandboxed (`ENABLE_APP_SANDBOX`, `Scrivi.entitlements`), which the Mac App Store and TestFlight require;
  `LSApplicationCategoryType` is set (productivity).

**Missing or not yet known:**
- ⚠️ **No privacy manifest** (`PrivacyInfo.xcprivacy`). The app calls APIs Apple lists as needing a stated reason:
  `UserDefaults` / `@AppStorage` (13 files), `ProcessInfo.systemUptime` (`HistoryCapture.swift:51`), and the C++ core's `stat` /
  `std::filesystem::last_write_time` (file timestamps: `LocalFileSystem.cpp`, `ProjectLock.cpp`, `ProjectArchive.cpp`, the
  `.docx` and Scrivener readers). App Store Connect checks uploads for these (Q3).
- ⚠️ **No export-compliance key** (`ITSAppUsesNonExemptEncryption`), so App Store Connect asks about encryption on every
  upload (Q2). Read from the code: the app has no network code and no encryption of its own (no `URLSession`, `CryptoKit`,
  `CommonCrypto`); the core hashes with SHA-256 (`util/Hash`) and zips with miniz (no encryption); secrets go to the Keychain.
- ⚠️ **The Mac's ScriviCore build names no architectures.** The Mac Run Script runs CMake for the host's architecture only
  (`project.pbxproj:685`); the iOS one passes `$ARCHS` (`:724`). An archive for "Any Mac" links every architecture Xcode
  builds for. If Xcode 27 builds only arm64 for macOS 27.1, nothing breaks; if it also builds x86_64, the link fails. ⏳ The
  first archive answers it.
- ⏳ **Never archived.** Nothing in the repo shows an archive or upload, so the first archive of each platform is the test.

**Outside the repo (yours, in App Store Connect):** the app record (both platforms), its name (must be unique on the store —
see Q4), the beta app description and feedback email for external testing, the tester groups.

## Goal

A tester invited by email installs Scrivi from TestFlight on an iPad, an iPhone and a Mac, and it opens, writes and autosaves as
your builds do. Each build in TestFlight names its commit.

## Plan

1. **Archive both platforms locally (Q1)** from a clean clone of the pushed commit (the stamp refuses a dirty tree), and fix
   only what the archives refuse — the Mac's architectures first, if they do.
2. **Privacy manifest (Q3):** `Scrivi/App/PrivacyInfo.xcprivacy`, in both app targets (pbxproj), each reason code taken from
   Apple's documentation (the `.json` route) for the call it covers; no data collected, no tracking.
3. **Export compliance (Q2):** `ITSAppUsesNonExemptEncryption` in `Info.plist` and `Info-iOS.plist`.
4. **Uploads (Q1, you):** Xcode ▸ Organizer ▸ the archive ▸ Validate App, then Distribute App ▸ TestFlight, for each platform.
5. **Testers (Q5).**
6. **Live passes (user):** install from TestFlight on the iPad, iPhone, Duo and Mac; Help ▸ About shows the commit.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Who archives and uploads.** An upload sends the app to Apple under your account; it cannot be taken back. | **I archive both platforms locally** (`xcodebuild archive`, from a clean clone of the pushed commit) and fix what they refuse; **you validate and upload from Xcode's Organizer.** Nothing leaves the Mac until you do. || ✅ As recommended (2026-11-20) |
| **Q2** | **Export compliance.** | **`ITSAppUsesNonExemptEncryption = NO`** in both Info.plists: the code has no encryption of its own and no network code; SHA-256 hashing, the system's Keychain and unencrypted zip are exempt. ⚠️ This is a legal declaration you make to Apple; the recommendation is only what the code shows. Without the key, App Store Connect asks on every upload. || ✅ As recommended (2026-11-20) |
| **Q3** | **The privacy manifest.** | **Add one now, declaring:** `UserDefaults` (the app's own settings), the system boot time (`systemUptime`, elapsed time between events) and file timestamps (the files the writer opens and the app's own); **no data collected, no tracking.** Each reason code from Apple's current list, cited in the Sprint, before it is written. || ✅ As recommended (2026-11-20) |
| **Q4** | **The name on App Store Connect.** It must be unique on the store, and testers see it; [T-0684] (*"too close to Scrivener"*) is still open. | **Use "Scrivi" now** and keep [T-0684] separate: the record's name can change before any release, and the name on the device is the bundle's. If "Scrivi" is taken, you choose one at record creation. || ✅ **Inkwyn** (user, 2026-11-20: *"I got it on the app store connect.  Inkwyn."*) — reserved in App Store Connect; the rename is [SP-229]. |
| **Q5** | **Testers.** External testers need Apple's Beta App Review for the first build of each version (usually about a day); internal testers (people on your App Store Connect team) do not. | **Internal first** (yourself, on every device), **then one external group** once the internal install passes, so a review wait never hides a build problem. || ✅ As recommended (2026-11-20) |

## Acceptance Criteria

- [ ] AC1 — The iOS and Mac apps archive from a clean clone of the pushed commit; each archive's version and build are the
      commit's (1.0.N, N) and its `ScriviCommit` the hash
- [ ] AC2 — A privacy manifest in both apps, every required-reason API covered by a documented reason (Q3)
- [ ] AC3 — Export compliance declared in both Info.plists (Q2)
- [ ] AC4 — Both archives pass Validate App and upload to App Store Connect (you, Q1)
- [ ] AC5 — An internal tester installs from TestFlight on the iPad, iPhone, Duo and Mac; an external group is invited and its
      build passes Beta App Review (Q5)
- [ ] AC6 — iOS tests, Mac interop, macOS and iOS builds, CI green

## Progress log

- **2026-11-20** — 🔵 **Created in Planning** (user, 2026-11-20: *"Let's to AC9 on EP-055"*), issued via `next-id.py`; carries
  [T-0690]. Q1–Q5 to rule.
- **2026-11-20** — ✅ **Q1–Q3, Q5 as recommended**; ⏸ **Q4 waits on [T-0684]** (user, 2026-11-20: *"with respect to the SP-228 questions.  Q1-Q3, and Q5 as recommended.  For Q4 I had already reserved Scrivi in my developer account.  But I think I want to do the renaming task first, before we go any further."*). The Sprint stays in Planning until the rename is done.

*Last Updated: 2026-11-20 (Q4 answered: **Inkwyn**; the Sprint waits on [SP-229] to close.)*

*Last Updated: 2026-11-20 (Q1–Q3, Q5 ruled; ⏸ waits on [T-0684].)*

*Last Updated: 2026-11-20 (🔵 **Created in Planning** (user, 2026-11-20: *"Let's to AC9 on EP-055"*).)*
