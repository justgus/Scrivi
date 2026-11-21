---
sprint: SP-229
epic: none
status: Closed
closed: 2026-11-20
platform: Cross
created: 2026-11-20
---

# SP-229 — `[Cross]` Rename the app: Scrivi → Inkwyn

**Status:** ✅ **CLOSED 2026-11-20** (user, 2026-11-20: *"Close SP-229"*). Was 🟡 **ACTIVE** (user, 2026-11-20: *"Q1-Q7 as recommended.  I accept our translation.  we may begin."*); ✅ Q1–Q7 as recommended, Q5 with our translation. ✅ **All ACs met** — live passes on the Mac, iPad, iPhone Duo and Linux VM (user, 2026-11-20: *"All steps pass on all platforms."*), CI green; awaiting close approval.
**Carries:** [T-0684].
**Epic:** none ([T-0684] has none). It comes before [SP-228] (TestFlight), which waits on it.
**Size:** M. Mostly strings and configuration. The Mac's app name and the test hosts are the parts that can break the build.

---

## What has been ruled

- ✅ **The name is Inkwyn** (user, 2026-11-20: *"I got it on the app store connect.  Inkwyn."*). It is *ink* + *wynn*: ᚹ (U+16B9)
  is the rune for "joy", which Anglo-Saxon scribes wrote as the letter ƿ until about 1300.
- ✅ **The rune appears in the logo only, never in the name** (user, 2026-11-20: *"we should onlt use the rune in the logo"*). Every
  piece of text, every file extension and every identifier stays plain ASCII.
- ✅ **The rune poem may be used** (user, 2026-11-20: *"We shouild be able to also use the (english translated) text of the wyn runic
  poem"*). That is the ᚹ stanza of the Old English Rune Poem (Q5).
- ✅ **Scope** ([T-0684], 2026-11-20):
  - **Renamed:** the visible name and the three file extensions.
  - **No compatibility** with the old extensions.
  - **Unchanged:** the bundle ID `com.caposoft.scrivi`, the Application Support folder, the `scrivi_*` ABI, the schema IDs, and the
    names of the repo and its files.

## What reading the code found (2026-11-20)

**The extensions** (`.scrivi`, `.scrivworld`, `.scrivarchive`):
- **Core:** `ProjectArchive.cpp:27–28`.
- **Apple:** `AppEnvironment.swift`, `ProjectArchiveFlow.swift`, `NewProjectSheet.swift`, `WorldsView.swift`, `ScriviURL.swift`, and
  each Info.plist's three document types (`UTTypeTagSpecification`).
- **Linux:** `main.cpp`, `ScriviWindow.cpp`, `EditorShell.cpp`, `WorldsDialog.cpp`, `SceneInspector.cpp`, `PackageFolderDialog.hpp`,
  `NewProjectDialog.qml`.
- **Tests:** 73 test files name `.scrivi` and 12 core tests name `.scrivworld`.
- The type identifiers (`com.caposoft.scrivi.project`, `.world`, `.archive`) are bundle-ID based, so they stay. Their descriptions
  ("Scrivi Project", "Scrivi World", "Scrivi Archive") change.

**The visible name:**
- **Apple:**
  - `CFBundleName` and `CFBundleDisplayName` (`Info.plist:8,16`; `INFOPLIST_KEY_CFBundleDisplayName` in 4 build configurations).
  - `PRODUCT_NAME = Scrivi` makes the file `Scrivi.app`, and Finder and the Dock show that file name (Q2).
  - About reads `CFBundleName` (`AboutView.swift:13`). The landing page hard-codes "Scrivi" (`LandingView.swift:42`).
  - About 45 more string literals name Scrivi across 15 files. Each one is sorted into visible text (renamed) or an identifier
    (kept).
- **Linux:**
  - `setApplicationName("Scrivi")` (`main.cpp:77`) also decides where settings and the session are stored, so it stays.
    `setApplicationDisplayName("Inkwyn")` is added beside it.
  - About 35 string literals across 12 files.
- **The module name is already separate:** `PRODUCT_MODULE_NAME = ScriviApp`. Renaming the product does not touch any `import` or
  `@testable import`.
- **The test hosts name the app's path:** `TEST_HOST = …/Scrivi.app/…` (`project.pbxproj:1246,1265,1284,1302`), and
  `scripts/run-interop-tests.sh:26` does the same.

**Linked worlds record their path:** a project stores each world's `lastKnownPath` (`WorldJson.cpp:82`). Renaming a world's folder on
disk therefore shows it as missing until **Locate…** is used once. That flow exists on both platforms.

## Goal

Every place a writer sees the app's name says **Inkwyn**: the app's file, the menus, About, the landing page, window titles,
dialogs, the Files and Finder kinds, and the Linux window manager. New projects, worlds and archives are saved as `.inkwyn`,
`.inkwynworld` and `.inkwynarchive`, and those are the only extensions the app opens. About shows the Wynn stanza.

## Plan

1. **Core:** the two extension constants, and the core tests' fixtures.
2. **Apple:**
   - Info.plists: names, the three type descriptions and the three extensions.
   - `PRODUCT_NAME`, the test hosts and the test script (Q2).
   - The visible string literals, sorted one by one.
   - About gets the stanza (Q5).
3. **Linux:**
   - `setApplicationDisplayName`, the visible strings, the extensions, the file dialogs' filters.
   - About gets the stanza (Q5).
   - Desktop entry and binary name: Q4.
4. **Tests:** every fixture and assertion that names an extension. A check that no visible "Scrivi" is left (AC5).
5. **Docs:** Q7.
6. **Your own projects:** rename their folders on disk (`*.scrivi` → `*.inkwyn`, `*.scrivworld` → `*.inkwynworld`), then Locate…
   each world once. No code does this (Q6).
7. **Live passes (you):** the Mac, the iPad, the iPhone Duo and the Linux VM.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **The three extensions.** | **`.inkwyn`, `.inkwynworld`, `.inkwynarchive`.** They follow today's pattern, and each says which app it belongs to. Shorter forms such as `.inkworld` lose the name. | ✅ As recommended (2026-11-20) |
| **Q2** | **The Mac app's file name.** Finder, the Dock and the Applications folder show the bundle's file name, not `CFBundleDisplayName`. | **`PRODUCT_NAME = Inkwyn` (`Inkwyn.app`)**, with the 4 `TEST_HOST` paths and `run-interop-tests.sh` updated. The module stays `ScriviApp`, so no Swift code changes for this. | ✅ As recommended (2026-11-20) |
| **Q3** | **Names inside a package:** `.scrivi-cache/`, `.scrivi-index/`, `*.scrivi-timeline.json`, the archive manifest `scrivarchive.json`, the staging folders `.scrivi-import-*` and `.scrivi-replaced-*`. | **Leave them.** They are file names inside a package, which the scope keeps. Changing them would be a format change with nothing visible to show for it. | ✅ As recommended (2026-11-20) |
| **Q4** | **Linux's installed names:** the binary `scrivi_linux` and the desktop file ID `scrivi` (`main.cpp:82`). | **Leave them for now.** No `.deb` exists yet. [EP-056] (packaging) names the package, the command and the `.desktop` file `inkwyn`, and this Sprint records that for it. The window manager still shows "Inkwyn" through `setApplicationDisplayName`. | ✅ As recommended (2026-11-20) |
| **Q5** | **Which translation of the Wynn stanza, and where it goes.** The Old English is about a thousand years old and free everywhere. Bruce Dickins's 1915 translation (*"Bliss he enjoys who knows not suffering, sorrow nor anxiety, and has prosperity and happiness and a good enough house."*) is public domain in the US. ⚠️ Dickins died in 1978, and in the UK and EU a work is protected for 70 years after its author's death. | **About, on every platform: the Old English stanza, with an English translation of our own beneath it.** You write or approve the wording; I can offer a close rendering to start from. That way no one else's rights are involved anywhere the app is sold. The other option is Dickins's text, credited, if you accept the UK/EU question. | ✅ As recommended (2026-11-20) — **our translation** (user: *"I accept our translation"*) |
| **Q6** | **Your existing projects and worlds,** including the tintagael and Eskandar test rig. | **Rename them by hand**, then Locate… each world once. That is about a minute's work, and it has no testers to serve ([T-0684]: no compatibility). The other option is a one-off script, kept out of the app, that also rewrites `lastKnownPath`. | ✅ As recommended (2026-11-20) |
| **Q7** | **Docs.** There are hundreds, and closed Sprints, Issues and Tasks record history. | **Change the current docs only:** CLAUDE.md, the READMEs, and the app-facing design docs' titles, each with a line saying "Inkwyn (formerly Scrivi)". The history stays as written, and the doc file names keep "Scrivi" (identifiers, by the scope). | ✅ As recommended (2026-11-20) |

**Out of scope:** the new **app icon with ᚹ**. It is artwork, so it gets its own Task, which I'll file when you rule. Until you supply
the art, today's icon stays. ✅ Filed as [T-0691] (2026-11-20).

## What was built (2026-11-20)

- **Extensions** — `.scrivi` → `.inkwyn`, `.scrivworld` → `.inkwynworld`, `.scrivarchive` → `.inkwynarchive` in 122 code files
  (core, Apple, Linux, tests, scripts), line for line; the bare forms in both Info.plists, `AppEnvironment.swift:864,873`,
  `ProjectArchiveFlow.swift:457`, `NewProjectSheet.swift:149–151`, `PackageFolderDialog.hpp:57`.
- **Apple** — `CFBundleName` / `CFBundleDisplayName` and the three type descriptions; `PRODUCT_NAME = Inkwyn` on the Mac, iOS and
  visionOS targets (Q2), each keeping its module name (`ScriviApp` on the Mac; `PRODUCT_MODULE_NAME = Scrivi` added on iOS and
  visionOS, so `@testable import Scrivi` is unchanged); the 4 `TEST_HOST` paths; `run-interop-tests.sh`. 21 visible strings.
- **Linux** — 17 visible strings (landing, About, window titles, file-dialog labels, import notes, `--version`); the
  `settings_smoke` menu path; the launcher's `Name=` in `Scrivi_Linux_Rig_Setup_v0_1.md` §5.
  ⚠️ **Deviation from the plan:** `setApplicationDisplayName` is **not** added. Every Linux window title is already set as text,
  and Qt appends the display name to each title, which would read "Inkwyn — Linux (alpha) — Inkwyn". The QMessageBox calls all
  pass a title.
- **About** (Q5) — `WynnStanza` (`AboutView.swift`): the Old English in italics, our translation, "— The Old English Rune Poem,
  wynn"; in the iOS sheet, the Mac's standard panel (centred, label colours for dark mode), and Linux's About dialog.
- **Docs** (Q7) — `CLAUDE.md`, `README.md`, `platforms/linux/README.md`, `Scrivi_Project_Package_Structure_v0_1.md`.

**Identifiers kept on purpose (AC5)** — each still says Scrivi and is never shown as the app's name:
- The bundle ID and type IDs `com.caposoft.scrivi*`; the `scrivi://` URL scheme (`Info.plist:93`, Spotlight deep links).
- The Application Support folder: `AppSupportLayout.cpp:41,65`, Linux `AppSupport.cpp:38`, `setApplicationName` (`main.cpp:77`,
  which also places settings and the session).
- The unnamed-author sentinel "Scrivi" (`AuthorStore.cpp:36–37`, `ProjectArchiveFlow.swift:519`, `ScriviBridge.cpp:1766`,
  `EditorShell.cpp:463`, `ScriviWindow.cpp:254`) — stored in projects, shown only as "Unnamed author".
- Log text: `[Scrivi]` prefixes, `~/Library/Logs/Scrivi`, the log headers (`ScriviDiag.swift:243,256`, `DiagnosticLog.cpp`).
- Linux's desktop file and icon name `scrivi`, the binary `scrivi_linux`, the QML module `Scrivi` (Q4).
- Names inside a package (Q3), the timeline file pattern `*.scrivi-timeline.json` (its label now says "Inkwyn Timeline").

**Tests (AC6, 2026-11-20):** core ctest **879/879** (macOS) · Mac interop **TEST SUCCEEDED** · iOS **38/38** · macOS build ·
Linux (Docker, Qt 6.4) ctest **884/884** as non-root and the CI's **43/43** smokes · `check-package-boundary`,
`check-abi-project-guards`, `check-engine-stub-parity`, `check-layout-convergence`, `check-manuscript-type-source`,
`check-textkit2` clean. ✅ CI green on `df64e04` (ScriviCore, Linux App, Apple).

## Live pass (AC7)

**A. Mac** (run from Xcode)
1. The menu bar reads Inkwyn: Inkwyn ▸ About Inkwyn, Hide Inkwyn, Quit Inkwyn. The Dock name is Inkwyn.
2. Inkwyn ▸ About Inkwyn: the version and commit, then the stanza — Old English in italics, the English beneath, "— The Old
   English Rune Poem, wynn". Readable in light and dark mode.
3. The welcome window is titled "Welcome to Inkwyn" and shows "Inkwyn".
4. File ▸ New Project…, name it "Rename Test". In Finder it is "Rename Test.inkwyn"; Get Info's Kind is "Inkwyn Project".
5. File ▸ Open Project…: the panel is titled "Open Inkwyn Project", and an old `.scrivi` cannot be chosen.
6. Quit. In Finder rename one of your projects `X.scrivi` → `X.inkwyn` and its world `Y.scrivworld` → `Y.inkwynworld` (Finder asks
   to confirm the extension: Use .inkwyn). Open the project; Manage Worlds… shows the world missing; Locate… it; it reconnects.
7. Export the project as an archive: the file is `.inkwynarchive`. Import it back; it opens.

**B. iPad and iPhone Duo**
8. The home-screen name is Inkwyn. The toolbar's About Inkwyn shows the stanza.
9. Create a project: in the Files app it is `….inkwyn`, Kind "Inkwyn Project", and tapping it opens it in Inkwyn.

**C. Linux VM** (pull, build, run)
10. The landing window is titled Inkwyn and shows "Inkwyn"; `scrivi_linux --version` prints "Inkwyn (Linux) …".
11. Help ▸ About Inkwyn shows the stanza, the Old English in italics.
12. A new project is saved as a `.inkwyn` folder. Rename a world folder to `.inkwynworld`, then Locate… it in Manage Worlds.
13. (Optional) the launcher: `sed -i 's/^Name=.*/Name=Inkwyn/' ~/.local/share/applications/scrivi.desktop ~/Desktop/scrivi.desktop`

## Acceptance Criteria

- [x] AC1 — The Mac app is `Inkwyn.app`. Its menus (About Inkwyn, Hide Inkwyn, Quit Inkwyn), About, landing page and window titles
      say Inkwyn. iOS's home-screen name, landing page and About say Inkwyn.
- [x] AC2 — Linux's window titles, dialogs, landing page and About say Inkwyn.
- [x] AC3 — New projects, worlds and archives are saved as `.inkwyn`, `.inkwynworld` and `.inkwynarchive`. The open dialogs on each
      platform show only those. Finder and Files describe them as "Inkwyn Project", "Inkwyn World" and "Inkwyn Archive".
- [x] AC4 — About shows the Wynn stanza as ruled in Q5, on the Mac, iOS and Linux.
- [x] AC5 — No visible "Scrivi" is left: a grep of the string literals, with each identifier kept listed by file:line in this
      Sprint.
- [x] AC6 — Core ctest, Mac interop, iOS tests, the macOS and iOS builds, the Linux smokes (Docker) and CI are green.
- [x] AC7 — Live passes: the Mac, the iPad, the iPhone Duo and the Linux VM. A renamed project and its world open after Locate….

## Progress log

- **2026-11-20** — 🔵 **Created in Planning** (user, 2026-11-20: *"plan the rename sprint"*), issued via `next-id.py`; carries
  [T-0684]. Q1–Q7 to rule.
- **2026-11-20** — 🟡 **ACTIVE** (user, 2026-11-20: *"Q1-Q7 as recommended.  I accept our translation.  we may begin."*): Q1–Q7 as recommended, our translation for Q5. Built the same day: AC1–AC5 met, AC6 green locally; T-0684 → 🟠; T-0691 filed (the ᚹ icon).
- **2026-11-20** — ⚠️ **Live pass, Mac step 4 failed** (user: *"When I tapped Create it said "the operation is not permitted"."*).
  **Cause:** ten stale `Scrivi.app` builds (DerivedData, the Trash, a Desktop probe build, `build-xc-*`) were still registered with
  LaunchServices, declaring the SAME type IDs (`com.caposoft.scrivi.project`, kept by the scope) with the OLD extensions. The
  system resolved the type to `.scrivi`, so the save panel enforced `.scrivi`; `NewProjectSheet.choosePath()` then rewrote the
  URL to `.inkwyn`, a path the sandbox had not granted → EPERM. **Fixed on the Mac:** each stale build unregistered
  (`lsregister -u`); `UTType("com.caposoft.scrivi.project")` now resolves to `inkwyn` (and `.world`/`.archive` likewise).
  ⚠️ Any old build that is launched or rebuilt re-registers the old tags. ⚠️ The rewrite after the panel can never work under the
  sandbox: it is the proximate failure, latent since I-0092. ✅ Filed as **[I-0366]** (user, 2026-11-20: *"yes, file the Issue"*), Issue backlog, not assigned.
- **2026-11-20** — ✅ **AC6, AC7 met**: every live-pass step passes on the Mac, iPad, iPhone Duo and Linux VM (user, 2026-11-20: *"All steps pass on all platforms."*) (step 4 after the stale registrations were removed); CI green on `df64e04`. T-0684 ✅ Verified. Awaiting close approval.
- **2026-11-20** — ✅ **CLOSED** (user, 2026-11-20: *"Close SP-229"*); T-0684 archived → `Tasks/Verified/Task-verified-0684.md`.

*Last Updated: 2026-11-20 (✅ **CLOSED** (user, 2026-11-20: *"Close SP-229"*).)*

*Last Updated: 2026-11-20 (✅ **All ACs met** (user, 2026-11-20: *"All steps pass on all platforms."*); awaiting close approval.)*

*Last Updated: 2026-11-20 (🟡 **ACTIVE** and built; ⏳ AC7 live passes.)*

*Last Updated: 2026-11-20 (🔵 **Created in Planning** (user, 2026-11-20: *"plan the rename sprint"*).)*
