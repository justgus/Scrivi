# Active Epics

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

**Status:** 🟡 **ACTIVE** 2026-10-24 — S0 ([SP-193]) ✅ closed 2026-10-25; no Sprint active — created 2026-10-24 (user: *"Lets start planning these features."*). ⚠️ The Epic's own Q1–Q6 are still to rule before S1.
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

**Questions (to rule):**

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | One Epic with a platform AC each (as [EP-052] was ruled), or one Epic per platform (`Epic-GUIDELINES.md`)? | **One Epic**: the formats live in the core and must be identical everywhere; each surface is a small AC. |
| **Q2** | **Formatted HTML for fanfic:** which sites (AO3, FanFiction.net, Wattpad, Tumblr …), and paste into a rich-text box or into an HTML field? | ⏳ **OPEN, the user is finding out** (2026-10-24). Known: *"it requires specific `<p></p>` tags for each paragraph."* So every paragraph becomes its own `<p>…</p>`; never `<br>` runs, never bare text. |
| **Q3** | **The archive:** a zip with its own extension, history always included? | **Yes:** one `.zip`-based file; history **included** (testers' real histories are the point); world optional, asked at export. |
| **Q4** | **Opening an archive of a project that already exists** on that machine (same project ID). | **Open it as a copy with a new ID**, named "… (imported)"; never replace. |
| **Q5** | **Order for the alpha.** | Archive + worlds first (testers move work and send you projects); then **HTML** and Markdown; then `.docx`; then PDF and ePub; **Scrivener import last** (the most complex format). |
| **Q6** | **Where each format is produced.** | In the core: Markdown, HTML, `.docx` and ePub (text and zipped XML). **PDF on each platform** from the HTML (PDFKit / Qt's PDF writer): page layout is a platform strength. |

**Provisional Sprints:** **S0 = [SP-193]** (✅ Closed 2026-10-25): fix before the alpha ([I-0306], [I-0313], [I-0202]) and one version everywhere ([T-0622]) · S1 archive + worlds (core) · S2 archive surfaces (Mac, Linux, iPad) · S3 Markdown + HTML export ·
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
