# Active Epics

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

---

## EP-048: `[Linux]` ⚠️ **Manuscript Renderer Parity**

**Status:** 🟡 **ACTIVE** since 2026-10-07 — S1, S2 and SP-173 closed; S3a closed (SP-174, 2026-10-10); S3b closed (SP-175, 2026-10-11); S4 (Find) closed (SP-176, 2026-10-11). L1–L7 and L9 met (S3b, 2026-10-10; Linux has no undo yet and no structured cross-scene paste — recorded in SP-175); L8 and L10 open — ✅ L8's Find half met (S4, [SP-176], rig live pass 2026-10-11); its Replace half is S4b ([SP-181], ✅ closed 2026-10-21). L10 is S5a–S5c: S5a ([SP-182]) ✅ closed 2026-10-21; S5b ([SP-183]) and S5c ([SP-184]) 🔵 Planning. ✅ **EP-048 stays open until EP-026 lands** (user, 2026-10-11) — ✅ EP-026 closed 2026-11-05; S5b ([SP-183]) and S5c ([SP-184]) remain.
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-165]** | **S1** — md4c analyzer in ScriviCore + `scrivi_analyze_markdown` + agreement test: L1, L2 (no rig) | ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-165.md`](../Sprints/Closed/Sprint-SP-165.md) · ✅ [T-0594] verified · ✅ L1, L2 met |
| **[SP-166]** | **S2** — the presenter: L3, L4, L5, L6 (⚠️ live pass needs the rig) | ✅ **CLOSED 2026-10-09** (user-approved) → [`../Sprints/Closed/Sprint-SP-166.md`](../Sprints/Closed/Sprint-SP-166.md) · ✅ L3–L6, L9 (part) met; [T-0595] verified |
| **[SP-173]** | `[Cross]` **I-0285** (found in S2): project load — no frozen UI, timeline built once, load measured; core calls synchronised (both platforms) | ✅ **CLOSED 2026-10-10** (user-approved) → [`../Sprints/Closed/Sprint-SP-173.md`](../Sprints/Closed/Sprint-SP-173.md) · ✅ I-0285 verified: load 172 s → 15.4 s |
| **[SP-174]** | **S3a** — `[Cross]` the command and balanced-edit logic moves into ScriviCore; Apple re-pointed (no visible change) | ✅ **CLOSED 2026-10-10** (user-approved) → [`../Sprints/Closed/Sprint-SP-174.md`](../Sprints/Closed/Sprint-SP-174.md) · ✅ Q1–Q5 ruled |
| **[SP-175]** | **S3b** — `[Linux]` the Format commands (L7) and balanced edits + copy (L9 rest) on the core logic; navigator toggle moves off Ctrl+Alt+N; rig live pass | ✅ **CLOSED 2026-10-11** (user-approved) → [`../Sprints/Closed/Sprint-SP-175.md`](../Sprints/Closed/Sprint-SP-175.md) · ✅ [T-0602] verified · ✅ L7, L9 met |
| **[SP-176]** | **S4** — `[Linux]` **Find** over the presented text: L8's Find half | ✅ **CLOSED 2026-10-11** (user-approved) → [`../Sprints/Closed/Sprint-SP-176.md`](../Sprints/Closed/Sprint-SP-176.md) · ✅ Q1–Q4 ruled; ✅ split ruled 2026-10-11 · ✅ rig live pass passed; [T-0603] verified · ✅ L8 Find half met |
| **[SP-181]** | **S4b** — `[Linux]` **Replace / Replace All** over the presented text: L8's Replace half — escaped, balanced, Replace All ONE undo step. ✅ Unblocked: Linux undo's grouped edit passed the rig ([SP-178], 2026-10-13) | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-181.md`](../Sprints/Closed/Sprint-SP-181.md) · ✅ Q1–Q3 ruled 2026-10-15 · ⚠️ `[Apple]` [I-0303] fixed first (Replace All writes off the main thread) · ✅ rig live pass 2026-10-21 |
| **[SP-182]** | **S5a** — `[Linux]` L10's character half: ✅ **builds Linux's Project Settings dialog** (replacing the *"coming soon"* stub; Project + Typography), the bundled typefaces and size, Markup Hints. EP-026 later adds its Undo History section | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-182.md`](../Sprints/Closed/Sprint-SP-182.md) · ✅ Q1–Q5 ruled 2026-10-15 (incl. the S5 split) · ✅ Q6 ruled 2026-10-21 (synthesized bold) · ✅ rig live pass 2026-10-21 |
| **[SP-183]** | **S5b** — `[Linux]` L10's block half: line spacing 1.45 and the first-line indent (Qt block formats — measured before the mechanism is ruled) | 🟡 **ACTIVE 2026-11-05** (re-planned the same day; ✅ built, AC0–AC4 met; ⏳ live pass) → [`../Sprints/Sprint-SP-183.md`](../Sprints/Sprint-SP-183.md) · ⏳ Q1–Q2 after its measurement |
| **[SP-184]** | **S5c** — `[Linux]` the **"Show chapter titles" toggle** ([EP-026] Q4) and the chapter-ending divider ([T-0575]) | 🔵 **PLANNING 2026-10-15** → [`../Sprints/Sprint-SP-184.md`](../Sprints/Sprint-SP-184.md) · ✅ Q1–Q2 ruled 2026-10-15 |

**Tasks:** ✅ [T-0594] (SP-165 — VERIFIED, archived) · ✅ [T-0595] (SP-166 — VERIFIED, archived) · ✅ [T-0601] (SP-174 — VERIFIED, archived) · ✅ [T-0602] (SP-175 — VERIFIED, archived) · ✅ [T-0603] (SP-176 — VERIFIED, archived) · ✅ [T-0608] (SP-181 — VERIFIED 2026-10-21, archived) · ✅ [T-0609] (SP-182 — VERIFIED 2026-10-21, archived) · 🔵 [T-0610] (SP-183) · 🔵 [T-0611] (SP-184).
**Issues:** ✅ [I-0285] (SP-173 — VERIFIED, archived) · ✅ [I-0280] (found + fixed in SP-165, Apple and core — VERIFIED, archived) · ✅ [I-0281] `[Apple]` (found by L2; fixed in [SP-169] — VERIFIED, archived).
This Epic is the escape layer's DISPLAY half on Linux; the WRITE half was [EP-049] (closed 2026-10-05). The design for both platforms was ruled in [SP-159].
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §9.

**Goal:** ✅ **The same manuscript surface on Linux.**

**Why:** `feedback_linux_adopts_apple_shape` — a shape change on Apple is made the same way on Linux. Where Qt cannot match
Apple's mechanism, the RESULT is matched and the difference recorded.

**Established (design: [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §2.4, §9):**
- The parser is **md4c inside ScriviCore**, reached through `scrivi_analyze_markdown` (Linux is a C ABI client); the core
  analyzer agrees with Apple's parser over the shared corpora (L2).
- The presenter is a `QSyntaxHighlighter` on `ManuscriptEditor` — presentation only; stored text and save bytes untouched (L3).
- ⛔ `QTextDocument::toMarkdown()` is NOT a save path (it re-wraps and writes escapes unescaped); Linux saves Apple's format.
- Measured differences, recorded: no hanging indent for list items (a block format is a document edit — ✅ accepted, SP-174 Q3);
  heading sizes are Apple's ratios over the body size.

| AC | Criterion |
| -- | --------- |
| **L1** | **A source-mapped Markdown analyzer in ScriviCore (md4c)**: per block → kind, markers, bold, italic, prefixes, spans; ⚠️ first measurement: how md4c reports escaped characters |
| **L2** | **Agreement test:** the core analyzer and Apple's `AttributedString` parser agree over the AC3 corpus + the S5 corpus (interop test) |
| **L3** | **The Linux presenter:** a `QSyntaxHighlighter` on `ManuscriptEditor`; the document's stored text and formats untouched; save bytes unchanged |
| **L4** | **Escape backslashes hidden** (Linux shows them today) with E1's rule; hard-break backslash per E1 AC6 |
| **L5** | **Hidden-run caret snap** (W3 measured the same invisible stops as Apple) |
| **L6** | **Headings + bold/italic render; re-entry span/line** (Q-E2-1) via `rehighlightBlock`; AC7 prose demotion |
| **L7** | **Commands** as EP-046 AC6 (Ctrl in place of ⌘) and Option/Alt-Return = hard break (Q-E2-4). ✅ **The Alt-Return STORAGE half is done** — matched in [SP-163] (2026-10-05) with the shared corpus, `escape_smoke` PASS |
| **L8** | **Find/Replace on presented text** (Q-E2-5) |
| **L9** | ✅ **ADDED 2026-10-07 (user, [SP-166] Q1)** — **Markers are atomic and edits stay balanced** (EP-046 AC12, design §3.6): ⌫/⌦ never delete a hidden marker alone (→ S2); balanced edits and copy without markers (→ S3, with the commands) |
| **L10** | ✅ **ADDED 2026-10-07 (user, [EP-047] P4)** — **[EP-047]'s preferences rendered on Linux**: the project typeface, the first-line indent (None · Every · Book convention), Markup Hints on/off — read from the shared `project-settings.json`. ⚠️ Qt's first-line indent is a BLOCK format (a document format, not a highlighter one — the SP-166 hanging-indent finding); measure it before ruling the mechanism |

Measured (SP-166): attach + first highlight of 1.8 MB ≈ 185 ms (Docker), a keystroke ~1 ms; the load as a whole → [SP-173].
