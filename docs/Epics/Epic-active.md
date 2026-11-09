# Active Epics

*Last Updated: 2026-11-09 (**EP-054: SP-206 (S2) → ✅ CLOSED** (user, 2026-11-09: *"close SP-206"*); archives move between the Mac, Linux and iPad. S3 not yet planned.)*

*Last Updated: 2026-11-06 (**EP-054: SP-205 (S1) → ✅ CLOSED** (user, 2026-11-06: *"yes, file the issue, then close SP-205."*); the archive endpoints exist, no surface yet.)*

*Last Updated: 2026-11-06 (**EP-054: SP-205 (S1) → 🟡 ACTIVE** (user: *"activate and implement SP-205."*).)*

*Last Updated: 2026-11-06 (**SP-206 (EP-054 S2) → 🟡 Active** (user, 2026-11-06: *"Q1–Q3 as recommended, activate and implement SP-206"*).)*

*Last Updated: 2026-11-06 (**EP-054 planned**: Q1–Q9 ruled as recommended; S1 = [SP-205], S2 = [SP-206] created in 🔵 Planning (user: *"You may complete the planning for EP-054."*).)*

*Last Updated: 2026-11-06 (**EP-048 → ✅ CLOSED** (user, 2026-11-06: *"F1–F5 as recommended, close EP-048."*) → `Closed/Epic-EP-048.md`.)*

## EP-057: `[Linux]` **Linux Rig Verification** — every Linux live pass waiting for the rig, held here and nowhere else

**Status:** 🟡 **ACTIVE 2026-11-02**, standing (user, 2026-11-02: *"1 and 2 approved"* — a standing rig Epic, and the Mac
pass plus Docker Linux evidence as the close gate). It never completes: it holds whatever rig passes are waiting.
**Rule:** [`../Sprints/Sprint-GUIDELINES.md`](../Sprints/Sprint-GUIDELINES.md) — *"The rig pass never holds a Sprint open"*.
**Steps:** [`../Sprints/Rig-Live-Pass-Held.md`](../Sprints/Rig-Live-Pass-Held.md), one section per Task below.

**Why:** the rig is away for some time; every Cross Sprint was held open on its rig pass while Mac, iOS and Linux kept
being built together. Development stays parallel; only the live pass on real Linux is verified here.

**Acceptance criteria (per Task, not per Epic):** the Task's held steps pass on the rig — ✅ **or the Linux VM on the Mac**
(user, 2026-11-02; not for x86_64, GPU or real-network ACs) — on a build that contains the work (Help ▸ About shows it), recorded in the Task; a failure is filed as a NEW Issue and linked from the Task.

**Tasks** (🔵 Backlog until a rig session runs them):

| Task | Rig pass | From |
|---|---|---|
| — | ✅ **None waiting.** [T-0661] passed on the VM 2026-11-04 (run 2) and is archived. | — |

---

---

## EP-054: `[ScriviCore]` **Import & Export** — projects, worlds and manuscripts in and out

**Status:** 🟡 **ACTIVE** 2026-10-24 — S0 ([SP-193]) ✅ closed 2026-10-25; **S1 = [SP-205] ✅ closed** 2026-11-06, **S2 = [SP-206] ✅ closed** 2026-11-09 — created 2026-10-24 (user: *"Lets start planning these features."*). ✅ **Q1–Q9 ruled 2026-11-06, every one as recommended** (user: *"I agree with your recommendations for all nine. I also agree that there should be two sprints."*).
**Codebase:** `[ScriviCore]` is home: the archive, the format writers and readers, behind `scrivi_*` endpoints. Each
platform's File menu / share sheet is its own AC (the [EP-052] precedent; see Q1).
**Goal:** a writer can **bring existing work in**, **take the manuscript out** in the formats they publish in (including
the formatted HTML the fanfic community posts), and **move a whole project, with its world, between machines**, or send
it with a bug report, without losing its history.

**What the code has today (read 2026-10-24):** no project, world or manuscript import/export at all. The only
import/export endpoints are `scrivi_import_external_timeline` / `scrivi_export_project_timeline`, `scrivi_import_asset`
and `scrivi_import_from_inbox`. A scene's text is Markdown (`.md`), so Markdown export is mostly assembly. [EP-032]
(inline object references, backlog) already records an **export gap**: references must resolve on export once they exist.

**Scope (draft):**
- **A — Project archive:** one file holding the whole `.scrivi` package (scenes, sidecars, timelines, assets, settings
  and **the history**), optionally with its bound world(s); opening it creates the project on the new machine.
- **B — Worlds:** a `.scrivworld` travels with the archive or on its own, and binds by world ID on arrival.
- **C — Manuscript export:** Markdown, **formatted HTML** (fanfic), Word `.docx`, PDF, ePub. The whole manuscript or
  chosen chapters/scenes; chapter titles and scene breaks as the writer has them.
- **D — Manuscript import:** Markdown / plain text, Word `.docx`, Scrivener `.scriv`, with headings and breaks mapped
  to chapters and scenes.
- **E — Surfaces:** File ▸ Import / Export on the Mac and Linux; Files and the share sheet on iPad/iPhone.

**Acceptance Criteria (draft):**
- [ ] AC1 — A project exported as an archive and opened on another machine reads, edits, undoes and redoes exactly as it
      did (history included), on Mac, iPad and Linux
- [ ] AC2 — An archive can carry the project's world(s); on arrival the world binds and its objects resolve
- [ ] AC3 — Opening an archive whose project already exists never overwrites it silently (Q4)
- [ ] AC4 — Export to formatted HTML that pastes and posts correctly on the fanfic sites the user names (Q2)
- [ ] AC5 — Export to Markdown, `.docx`, PDF and ePub (order per Q5), with chapters, scene breaks and emphasis preserved
- [ ] AC6 — Import from Markdown / text, `.docx` and Scrivener, with chapters and scenes mapped and nothing dropped
      silently (what can't be mapped is reported)
- [ ] AC7 — Every format boundary-tested through `scrivi_*`, with round-trip fixtures
- [ ] AC8 — The surfaces on Mac, iPad/iPhone and Linux, each live-passed on its platform

**Questions — ✅ all nine ruled as recommended (user, 2026-11-06):**

| # | Question | Ruling |
| - | -------- | -------------- |
| **Q1** | One Epic with a platform AC each (as [EP-052] was ruled), or one Epic per platform (`Epic-GUIDELINES.md`)? | **One Epic**: the formats live in the core and must be identical everywhere; each surface is a small AC. |
| **Q2** | **Formatted HTML for fanfic:** which sites (AO3, FanFiction.net, Wattpad, Tumblr …), and paste into a rich-text box or into an HTML field? | ✅ **Every paragraph becomes its own `<p>…</p>`**; never `<br>` runs, never bare text (user, 2026-10-24: *"it requires specific `<p></p>` tags for each paragraph."*). ⏳ **Which sites, and rich-text box vs HTML field, still to name — before S3** (S1–S2 do not need it). |
| **Q3** | **The archive:** a zip with its own extension, history always included? | **Yes:** one `.zip`-based file; history **included** (testers' real histories are the point); world optional, asked at export. |
| **Q4** | **Importing an archive whose project folder name already exists in the chosen folder.** | ✅ **Re-ruled 2026-11-06** (user): treat it like saving over a file — **Cancel** (nothing is extracted), **Rename** (the project folder is unpacked under the new name, as the `dumas-prose` / `dumas-prose-timelines` / `dumas-prose-timelines-history` fixtures already are) or **Replace** (the old project folder and all its files are deleted and the archive's put in its place). The project **keeps its ID** — an ID identifies the work, not the folder (user: *"Project IDs exist so that we can determine if two project folders are copies or versions of one another"*). ⛔ The earlier "copy with a new ID, named … (imported)" is withdrawn. |
| **Q5** | **Order for the alpha.** | Archive + worlds first (testers move work and send you projects); then **HTML** and Markdown; then `.docx`; then PDF and ePub; **Scrivener import last** (the most complex format). |
| **Q6** | **Where each format is produced.** | In the core: Markdown, HTML, `.docx` and ePub (text and zipped XML). **PDF on each platform** from the HTML (PDFKit / Qt's PDF writer): page layout is a platform strength. |
| **Q7** | ➕ *(re-plan 2026-11-06)* **The zip library.** ScriviCore has none (its only dependencies are nlohmann/json and md4c), and the archive, `.docx` and ePub are all zip files. | **miniz**, pinned by FetchContent and compiled straight into the static library as md4c is (one `.c` file, MIT, no system dependency), so `libScriviCore.a` stays the app's single link input on every platform. Alternative: the system zlib + minizip (no zlib on iOS without a bundled copy) |
| **Q8** | ➕ *(re-plan 2026-11-06)* **Where an arriving world goes.** A project's binding records the world's path on the SENDING machine, and a world is a separate package, often shared by several projects. | If a world with the same world ID is already bound or found on this machine, **bind to it and unpack nothing**; otherwise ask where to put it (default: beside the project), unpack it there and `scrivi_relink_world` the binding. Never a second copy of a world the machine already has |
| **Q9** | ➕ *(re-plan 2026-11-06)* **What the archive leaves out.** | **Out:** `.lock` files, `.cache/`, `indexes/`, OS noise (the package spec's §17 list) — all rebuilt on open. **In:** everything else, including `history/` (Q3) and `.git/` when Git snapshots are on (it IS the snapshot history `snapshots/` points at). The open project's write queue is drained first ([EP-052]), so the archive is never a half-written state |

**Re-read 2026-11-06** (user: *"plan EP-054"*): still no project, world or manuscript import/export (`scrivi.h`). ⚠️ **No zip
library in ScriviCore** (→ Q7). The archive's contents are the package as laid out in `Scrivi_Project_Package_Structure_v0_1.md`
— `history/` already lives inside it (§16a, Trade T6, so undo travels by design) and is excluded only from Git (§17). A world
binding holds the sending machine's path (`worlds/`, §11.2) and a world is its own package with its own `.lock`
(`WorldStore.cpp:75`) (→ Q8).
**S1 = [SP-205] (core only, no surface):** `scrivi_export_archive(projectRoot, destPath, includeWorlds)` and
`scrivi_import_archive(archivePath, destFolder, …)` → the unpacked project's root, which the platform then opens; Q4's Cancel / Rename / Replace, Q8's world binding, Q9's contents;
boundary tests through `scrivi_*` with a round trip whose undo/redo replays identically (AC1–AC3 at the core, AC7's first
fixtures). **S2 = [SP-206]** then puts File ▸ Export Archive… / Import Archive… on the Mac, Linux and iPad.

**Sprints** (S3–S7 provisional): **S0 = [SP-193]** (✅ Closed 2026-10-25): fix before the alpha ([I-0306], [I-0313], [I-0202]) and one version everywhere ([T-0622]) · **S1 = [SP-205]** archive + worlds (core, [T-0664]) ✅ Closed 2026-11-06 · **S2 = [SP-206]** archive surfaces (Mac, Linux, iPad, [T-0665]) ✅ Closed 2026-11-09 · S3 Markdown + HTML export ·
S4 `.docx` export · S5 PDF + ePub · S6 Markdown/text + `.docx` import · S7 Scrivener import. Each with live passes.
**Depends on:** nothing open. ⚠️ [EP-055] needs S1–S2 on iPad to move projects onto it, unless iCloud Drive does (see
EP-055 Q2).

---

## EP-055: `[Apple]` **iPad and iPhone** — the manuscript first, then the full app on iPad and a phone surface

**Status:** 🟡 **ACTIVE** 2026-10-29 — S1 = [SP-198] ✅ closed 2026-11-03; S2 = [SP-199] ✅ closed 2026-11-03 (panels by size class, gestures, toolbar); no Sprint active. Created 2026-10-24 as a Draft; activated 2026-10-29 (user:
*"Activate EP-055"*), with the manuscript ruled first (user, 2026-10-29: *"In the iOS version, we should focus on the
Manuscript View first."*).
**Codebase:** `[Apple]` (`Scrivi/`, the `ScriviApp-iOS` target). The core is shared and already cross-builds for iOS.
**Goal:** a writer can **write on, or simply read on,** a real project on an **iPad**, an **iPhone** and the **iPhone Duo**
(user: *"On iOS it should be wonderful to write on, or simply read on."*), with the on-screen keyboard or a hardware
keyboard; then the rest of the app on iPad (navigator, inspector, timeline, worlds, undo history, buffers) and a phone
surface, all delivered by **TestFlight**.

**What the code has today (read 2026-10-29):**
- ✅ A `ScriviApp-iOS` target, now at **27.1** (set by the user; [SP-198] R1), and a build phase that cross-builds ScriviCore for the
  iOS SDK (`build-${PLATFORM_NAME}`). ✅ It builds for the iOS Simulator under Xcode 27.2.
- ⚠️ **The manuscript editor on iOS is a stub:** *"Manuscript editor not yet available on this platform."*
  (`ManuscriptTextView.swift:3710`). The Mac editor is 3,718 lines of AppKit: the `NSViewRepresentable` (`:37`), its
  `Coordinator` (`:287`, ~2,300 lines: history, structural edits, boundary navigation, buffers, cross-scene cut/copy/paste)
  and `ManuscriptNSTextView` (`:2614`: key bindings, Find, the escape layer, Format commands, undo routing).
- ✅ **The presentation layer is already TextKit 2,** which UIKit shares: `ManuscriptPresenter` is an
  `NSTextContentStorageDelegate` / `NSTextStorageDelegate`; its AppKit dependency is mostly `NSFont` / `NSColor`. It,
  `ManuscriptPresentedMap`, `ManuscriptFind` and `ManuscriptAccessibility` sit inside `#if os(macOS)`. `MarkdownBlocks`,
  `MarkdownEmphasis` and `ManuscriptEscapes` are already platform-neutral.
- The porting outline (`Scrivi_Platform_Porting_Outline_v0_1.md` §7) puts iPad 2nd and iPhone 3rd, and warns that **§4.7
  (no gesture-only affordance) inverts** on touch.

**The iPhone Duo (Apple's documentation, fetched 2026-10-29; all iOS 27.1+):**
- `UIHingeInteraction` reports a `UIHinge`: `angle` (**radians**) and `status` (`.closed`, `.partiallyOpen`, `.fullyOpen`,
  `.unknown`); `update.hinge == nil` means the view left a hierarchy that has a hinge.
- `UIView.reservedRegions(kind:options:)`: **the fold is a `.division`**, *active* when the device is partially open or
  closed, *inactive* when fully open.
- *"Preparing your app for iPhone Duo"*: prefer system containers; lay out by size class, **never by
  `userInterfaceIdiom`** (⚠️ `EditorView.swift:406,475` do that today, outside the manuscript).
- ⛔ **None of this is used** until the user identifies a need (Q8).

**Scope:** the manuscript editor on UIKit (TextKit 2) sharing the presenter and Markdown model, with the system's default
keyboard and layout behaviour (Q8); an iOS test target; then navigator, inspector card stack, timeline, worlds and detail sheets; undo/redo
with the session question, the branch chooser and mark; buffers and keyboard shortcuts; Find; project settings; file
access (Files, iCloud Drive, external drives on iPad); iPad multi-window; a phone layout; TestFlight delivery.

**Acceptance Criteria:**
- [x] AC1 — ✅ [SP-198] (closed 2026-11-03) — A gap audit of the manuscript editor's host: every behaviour in `ManuscriptTextView.swift`, AppKit-bound or
      shareable, and the iOS shape ruled (Q3). The rest of the surfaces audited before S3.
- [x] AC2 — ✅ [SP-198] (closed 2026-11-03) — **The manuscript on iPad and iPhone:** a real project reads and scrolls well, and the writer types, selects and
      autosaves, with the markers and hints, chapter titles and scene dividers as on the Mac
- [x] AC3 — ✅ [SP-198] (closed 2026-11-03) — The manuscript works with the on-screen keyboard and a hardware keyboard on iPad, iPhone and the Duo, using the
      system's default behaviour and layout (Q8): nothing device- or keyboard-specific
- [x] AC4 — ✅ [SP-198] (closed 2026-11-03) — An iOS test target, run on the simulator, covering the manuscript's behaviour
- [ ] AC5 — Structural edits, undo/redo and Find in the manuscript on iOS — ⚠️ not yet on iOS (user, 2026-11-01): **Find and
      Replace**, and **Go to Scene / Chapter / Manuscript Begin and End**
- [ ] AC6 — Navigator, inspector, timeline, worlds and detail sheets on iPad
- [ ] AC7 — Projects opened from, and kept on, the places the testers use (Q2), surviving app relaunch
- [ ] AC8 — The phone surface, per Q1's feature set
- [ ] AC9 — TestFlight builds for iPad/iPhone (and the Mac, per the delivery ruling), installable by external testers
- [ ] AC10 — Live passes on a real iPad, a real iPhone and the Duo (simulator until the device arrives, Q6)

**Questions:**

| # | Question | Ruling / Recommendation |
| - | -------- | ----------------------- |
| **Q1** | **The phone's feature set**, and the Duo's layouts. | ✅ **Partly ruled:** the manuscript first on every device (user, 2026-10-29); the Duo by size class only, nothing special (Q8). Recommendation for the rest: write, navigate, undo, Find and the inspector on the phone; the timeline and world management iPad-only at first; the unfolded Duo laid out by size class. |
| **Q2** | **Where projects live on iPad/iPhone,** and how they get there from a Mac. | **Files / iCloud Drive** (a project folder opened through the document picker, kept by a security-scoped bookmark as the Mac does), plus the [EP-054] archive. CloudKit sync stays future. |
| **Q3** | **The editor:** port the AppKit editor to UIKit, or first extract a platform-neutral editing core? | **Share the presentation layer** (TextKit 2 is common to both); a new `UITextView` host. What else moves out of the Mac `Coordinator` is [SP-198]'s audit ([T-0632]) to rule. |
| **Q4** | **One Epic or two?** | **One Epic now**; split after AC1 if the editor proves to be an Epic of its own. |
| **Q5** | **TestFlight prerequisites.** | ✅ **RULED (user, 2026-10-24): *"yes, I am all setup for testflight."*** Builds are Debug (pre-release). |
| **Q6** | **Test devices.** | ✅ **RULED (user, 2026-10-29):** a real iPad and iPhone now; *"Duo simulator only for now"*, a real Duo in about two weeks. |
| **Q8** | **Device- and keyboard-specific handling** (the Duo's fold, keyboard avoidance). | ✅ **RULED (user, 2026-10-30): no special handling for the Duo or for keyboards.** *"stay using only the size classes and don't do anything special for iPhone Duo until I identify a need … don't do anything special with regard to keyboards on either the iPhone or iPad as well. The default layout will be sufficient … iOS also detects when a hardware keyboard is in place and acts accordingly."* Layout by size class only; toolbar buttons at their default placement. |
| **Q7** | **The iOS deployment target.** | ✅ **RULED (user, 2026-10-29): 27.1**, set by the user. |

**Provisional Sprints:** **S1 = [SP-198]** the audit, the shared presenter, the `UITextView` host and the iOS test target
(AC1–AC4) · S2 structural edits, undo/redo, Find (AC5) · then AC6–AC9 as the audit orders them. Each with live passes.
**Depends on:** [EP-054] S1–S2 (the archive) for moving projects onto an iPad, unless Q2's iCloud Drive route is enough
(not needed for S1: the iOS Landing view already opens a project through `.fileImporter`, `LandingView.swift:84`).
