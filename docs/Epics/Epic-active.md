# Active Epics

## EP-052: `[ScriviCore]` **Background Writes** — one ordered write queue per project, in the core

**Status:** 🟡 **ACTIVE 2026-10-26** (user, 2026-10-26: *"close SP-188 and activate EP-052."*) — created 2026-10-15 (user: *"A new Epic."*, ruling Q5 of the design). S1 [SP-194] ✅ closed 2026-10-26 (AC0–AC3). S2a [SP-195] 🟡 active 2026-10-26; S2b [SP-196] 🟡 active 2026-10-27 (beside S2a, whose rig pass is open). S3 [SP-197] 🟡 active 2026-10-27.
**Design:** [`../Scrivi_Background_Writes_Design_v0_1.md`](../Scrivi_Background_Writes_Design_v0_1.md) — ✅ Q1–Q5 ruled
2026-10-15. **Builds on:** `Scrivi_Core_Concurrency_Design_v0_1.md` (SP-173's per-project lock).
✅ **v0.2 RULED 2026-10-26** (user: *"Q1–Q7 agreed as recommended"*): [`../Scrivi_Background_Writes_Design_v0_2.md`](../Scrivi_Background_Writes_Design_v0_2.md) —
the main thread is for the screen: the main-thread inventory (Mac + Linux), two locks per project (memory and disk),
reads off the main thread, Spotlight in the background. Q4 (objects cached in memory) and Q6 (level 5) are not now.
**Codebase:** `[ScriviCore]` is home (the queue, the endpoints, the ordering rule). Apple and Linux adopt it in the SAME
Sprint (`feedback_linux_adopts_apple_shape`), each AC verifiable on its own platform.
⚠️ `Epic-GUIDELINES.md` prefers one Epic per platform; the user ruled ONE Epic (Q5), so the two adoptions are separate ACs.

**Goal:** no write to a project, including undo, redo, Replace All, structural operations and the typing autosave, runs on
the GUI thread. Every write to one project lands in the order it was made, by ONE rule in ScriviCore, and the editor never
locks to wait for a save.

**Why:** undoing a large Replace All writes every scene on the GUI thread (SP-181's finding); every typing autosave is a
network write on the GUI thread over the share; and the two platforms each keep their own rule to stop an older write landing
over a newer one, which have already drifted once ([I-0304] against [I-0259]).

**Acceptance Criteria:**
- [x] **AC0 — `[ScriviCore]` Two locks per project (v0.2 Q1):** a memory lock (M) and a disk lock (D) replace SP-173's
  single exclusive lock; D before M, never M before D; an endpoint that touches no file takes M only.
- [x] **AC1 — `[ScriviCore]` The queue (W1, W3):** one writer thread per open project, first-in-first-out, coalescing a scene
  queued again before its write starts; every synchronous endpoint that reads or writes scene files first waits for the
  jobs queued before it; `scrivi_close_project` drains it.
- [x] **AC2 — `[ScriviCore]` The endpoints (W2, W4, W7):** queue scene saves and structural operations with a ticket; the
  history log's appends (undo, redo, capture) join the same queue; the core tells history the persisted bytes itself;
  `scrivi_wait_for_writes` reports every outcome, a failure included.
- [x] **AC3 — `[ScriviCore]` Tests:** boundary tests through `scrivi_*` (order, coalescing, read-after-queue, failure, close
  drains) and a ThreadSanitizer stress run on macOS and Linux.
- [ ] **AC4 — `[Apple]` Adoption:** autosave, Replace All, undo and redo writes, and structural operations go through the
  queue; no project write on the main thread; the manuscript does not lock for a save (Q3); a failed write leaves its scene
  dirty; close and quit wait ([I-0305]'s dialog). W6's platform-side ordering code is removed.
- [ ] **AC5 — `[Linux]` Adoption:** the same, in the same Sprint as AC4.
- [ ] **AC6 — Rig:** undoing a Replace All on dumas over the share does not freeze the window, and typing never waits on a save.
- [ ] **AC7 — `[Linux]` Replace All shows its progress** (moved from [SP-185] AC5, user 2026-10-22: *"Move AC5 to EP-052"*):
  [SP-185] Q3 as ruled 2026-10-17 — a real count (*"Replacing 312 of 2,048"*), and the view follows a few times a second,
  showing replacements land. To be designed against the queue (it changes Replace All's locked-editor shape).
- [ ] **AC8 — `[Apple]` + `[Linux]` Reads off the main thread (v0.2 Q2, R3):** the Inspector and the timeline load as
  one read job each; a result whose revision moved is dropped; a click, select or scroll makes no file operation on the
  main thread (the inventory probes read 0).
- [ ] **AC9 — `[Apple]` Spotlight in the background (v0.2 Q3):** one serial queue, per-scene re-index, behind the saves;
  no Spotlight work on the main thread at open or on app switch.
- [ ] **AC10 — Gesture cues (v0.2 Q5):** every queued gesture shows a cue at once; a repeat is ignored while one is
  pending; ⌘Z waits its turn and never cancels a write.
**Rough span (v0.2 Q7):** 4 Sprints (S2 split in two, ruled 2026-10-26, [SP-195] Q2): S1 ScriviCore (AC0–AC3), S2a every save queued (AC4–AC7), S2b structural edits queued + cues (AC4, AC5, AC10),
S3 Apple + Linux reads off the main thread + Spotlight (AC8, AC9). Each ends with a live pass over the loop share.
**Backlog, not yet an AC:** [T-0628] keep a held queue on local disk and push it when the project returns (user, 2026-10-26).
**Out of scope:** world writes (concurrency design D6), two processes on one project, the speed of the structural
operations themselves.

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

## EP-053: `[Cross]` **Range-Based Structural Editing** — cut, copy, paste and undo as one Range, one core call, no reload

**Status:** 🟡 **ACTIVE 2026-10-17** — created and started the same day (user: *"draft EP-053, start the first sprint and publish
the design doc for my perusal"*). S1 = [SP-186], ✅ closed 2026-10-20. S2a = [SP-187], ✅ closed 2026-10-21. S3 = [SP-188], ✅ closed 2026-10-26; no Sprint active.
**Design:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_3.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_3.md) (v0.3, [SP-191]; supersedes v0.2) — ✅ ruled
2026-10-17 (R1–R7, Q1–Q5). Supersedes v0.1 (EP-029) where they differ.
**Codebase:** `[ScriviCore]` first, then `[Apple]` and `[Linux]` together — the same shape and the same threads on both (R5).
**Absorbs:** [I-0297] (a paste's undo takes 40 s over the share — its last part, the reload), [I-0311] (a chapter-opening paste
scrambles the reading order), and the structural half of [SP-185].
**Relates to:** [EP-052] (Background Writes): once EP-052's per-project write queue exists, EP-053's structural calls run on
it; until then each platform runs them off its main thread itself (S2).

**Goal:** ✅ A cut, copy, paste or undo across scenes and chapters touches only the files in its range — a two-scene paste is
4 reads and 4 writes — is one ScriviCore call, never reloads the manuscript, and behaves and performs the same on the Mac and
on Linux, over a network share as on a local disk.

**Why (measured 2026-10-17, 1,186 scenes):** a copy read every scene sidecar (2,678 file calls); a paste re-read its chapter
three times (184); both apps re-opened the whole project after every paste (~1,700 + every scene text — the Mac on its main
thread, one core call per scene); a cut no longer removed structure, so paste and cut were not inverses. Design §2–§3.

**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-186]** | **S1** — `[ScriviCore]` the Range (`scrivi.range.v1`) and the range operations: read, paste ([I-0311] fixed), structural cut, each returning the delta; the index patched in place; a two-scene paste = 3 reads + 5 writes (re-ruled 2026-10-20) | ✅ **CLOSED 2026-10-20** (user-approved) → [`../Sprints/Closed/Sprint-SP-186.md`](../Sprints/Closed/Sprint-SP-186.md) · [T-0614] verified |
| **[SP-187]** | **S2a** — `[Apple]` + `[Linux]` the scene list as the model of record (design §4.2a), no visible change: the range endpoints bound on both, the delta types, Linux's list holding the text, Apple's partial re-render; baseline counts | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-187.md`](../Sprints/Closed/Sprint-SP-187.md) · ✅ Q1–Q4 ruled |
| **[SP-191]** | **S2b** — `[Apple]` + `[Linux]` copy builds the Range from memory; paste and its undo (a range cut); split, chapter split, merge and chapter merge as range calls (undoable, sidecars restored, no barriers); the delta applied, no reload, off the main thread on both; old history nodes converted; [I-0314] · ➡️ takes [SP-185] AC4 | ✅ **CLOSED 2026-10-22** (user-approved) → [`../Sprints/Closed/Sprint-SP-191.md`](../Sprints/Closed/Sprint-SP-191.md) · [T-0619] verified |
| **[SP-188]** | **S3** — `[Apple]` + `[Linux]` structural cut; Backspace / Delete over scenes = Cut minus the clipboard (Q4); cut's undo (a range paste); old `structuredCut` nodes still undo | ✅ **CLOSED 2026-10-26** (user-approved) → [`../Sprints/Closed/Sprint-SP-188.md`](../Sprints/Closed/Sprint-SP-188.md) · AC11's Mac pass over a share → [SP-190] |
| **[SP-189]** | **S4** — `[Cross]` the background sync verification (disk wins, silently — Q3) | 🔵 **PLANNING 2026-10-20** → [`../Sprints/Sprint-SP-189.md`](../Sprints/Sprint-SP-189.md) · ⏳ Q1–Q2 |
| **[SP-190]** | **S5** — Mac and rig live passes over the share, timed by the diagnostic logs | 🔵 **PLANNING 2026-10-20** → [`../Sprints/Sprint-SP-190.md`](../Sprints/Sprint-SP-190.md) · ⏳ Q1 |

**Planning rulings (user, 2026-10-20):** ✅ each operation ships with its undo (paste + its undo in S2, cut + its undo in
S3; verification moves to S4, history no longer a Sprint of its own) · ✅ split, merge and chapter create/merge are in scope,
in S2 · ✅ (later the same day) S2 split into S2a [SP-187] and S2b [SP-191]; the list stays an ordered array (Q4) · ✅ new history op names `rangePaste` / `rangeCut` in `scrivi.history.v1`, backward-compatible (old
`structuredPaste` / `structuredCut` nodes keep undoing).

**Tasks:** ✅ [T-0614] (SP-186 — VERIFIED, archived) · ✅ [T-0615] (SP-187 — VERIFIED 2026-10-21, archived) · ✅ [T-0619] (SP-191 — VERIFIED 2026-10-22, archived) · 🟠 [T-0616] (SP-188 ✅ closed — Implemented - Not Verified, ⏳ the Mac pass over a share in SP-190) · 🔵 [T-0617] (SP-189) · 🔵 [T-0618] (SP-190).
**Issues:** ✅ [I-0311] · ✅ [I-0297] · ✅ [I-0314] (all VERIFIED 2026-10-22, archived, with [SP-191]).

---

## EP-050: `[Apple]` ⚠️ **Manuscript Accessibility** — VoiceOver reads the page the writer reads

**Status:** 🟡 **ACTIVE 2026-10-08** (SP-171 activated) — created 2026-10-08 (user: *"let's look at I-0277"*; rulings A1–A3 the same day). ✅ S1 = [SP-171], ✅ closed 2026-10-08; S2 = [SP-172], ✅ closed 2026-10-09.
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-171]** | **S1** — the translation layer (one map, shared with Find), the caret both ways, cost: AC1, AC2, AC3 + AC5 reading half | ✅ **CLOSED 2026-10-08** (user-approved) → [`../Sprints/Closed/Sprint-SP-171.md`](../Sprints/Closed/Sprint-SP-171.md) · Q2 re-ruled: dividers read as WORDS · ✅ AC1, AC2, AC3, AC5 reading half met |
| **[SP-172]** | **S2** — the Headings rotor: AC4 + AC5 rotor half | ✅ **CLOSED 2026-10-09** (user-approved) → [`../Sprints/Closed/Sprint-SP-172.md`](../Sprints/Closed/Sprint-SP-172.md) · rulings R1–R3 there · ✅ AC4 + AC5 rotor half met (live pass 2026-10-09); T-0600 verified |
| S3 | Linux: AC6 (measure Orca, then rule) — ⚠️ needs the rig | not yet created |

**Tasks:** ✅ [T-0599] (SP-171 — VERIFIED, archived) · ✅ [T-0600] (SP-172 — VERIFIED, archived).
**Issues:** ✅ [I-0277] `[Apple]` VoiceOver reads the STORED manuscript — fixed and VERIFIED in [SP-171], archived. · 🔵 [I-0284] (found in SP-171 → Issue backlog).
**Authority:** design [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §13 (*"VoiceOver /
accessibility reads storage. Hidden markers may be spoken; not checked."*); SDK `AppKit/NSAccessibilityProtocols.h`,
`NSAccessibilityCustomRotor.h` (read 2026-10-08).

**Goal:** ✅ **A writer using VoiceOver hears the manuscript as it reads on the page** — no backslashes, no `##`, no `**` — and can
move from heading to heading the way she would on the web.

### ⚠️ What the SDK says the work is (read 2026-10-08)

A text view speaks to assistive technology through **~15 members, every one in CHARACTER POSITIONS**: `accessibilityValue`,
`accessibilityNumberOfCharacters`, `accessibilitySelectedText`, `accessibilitySelectedTextRange(s)` (get AND set — VoiceOver moves the
caret through them), `accessibilityVisibleCharacterRange`, `accessibilityInsertionPointLineNumber`, `accessibilityLine(for:)`,
`accessibilityRange(forLine:)`, `accessibilityString(for:)`, `accessibilityAttributedString(for:)`, `accessibilityRange(forPosition:)`,
`accessibilityRange(for index:)`, `accessibilityFrame(for:)`, `accessibilityRTF(for:)`. ⛔ So the fix is not "a different string": it is a
**TRANSLATION LAYER** between the PRESENTED text and STORAGE, consistent across every member — ⚠️ or VoiceOver's cursor and Scrivi's
caret drift apart (the [I-0282] lesson). ✅ The pieces exist: `MarkdownEscapes.map`, the presenter's `stopTest` / `isHidden`, the
caret-snap rules. ✅ `NSAccessibilityCustomRotor` (macOS 10.13+) has `NSAccessibilityCustomRotorTypeHeading` and a text result's
`targetRange` — the Headings rotor.

### ✅ Rulings — 2026-10-08 (user)

| # | Question | Ruling |
| - | -------- | ------ |
| **A1** | What text VoiceOver gets | ✅ **The page AT REST** — every Markdown mark hidden (escapes, heading `#`, emphasis markers) whatever the caret or Markup Hints; list prefixes kept, as on screen. Stable as the caret moves |
| **A2** | Headings | ✅ **A Headings rotor** — VoiceOver moves heading to heading (and chapter titles) |
| **A3** | Tracking | ✅ **This Epic**; [I-0277] its first Issue; Linux its own criterion |

### Acceptance criteria

| AC | Criterion |
| -- | --------- |
| **AC1** | **One translation layer (A1):** the accessibility text is the page at rest; EVERY member above maps presented ⇄ storage through ONE map; a corpus test proves round trips and agreement between members (e.g. `string(for: r)` = the substring of `value` at `r`; `line(for:)` / `range(forLine:)` agree) |
| **AC2** | **The caret, both ways:** a selection set by VoiceOver lands where Scrivi's caret rules put it (never inside hidden markup); Scrivi's caret is reported in presented positions |
| **AC3** | **Cost:** the map is cached and invalidated per edit — measured on 1.8 MB (`numberOfCharacters`, `string(for:)`, a keystroke with VoiceOver's queries); recorded |
| **AC4** | **The Headings rotor (A2):** Markdown headings and chapter titles, in order, each a `targetRange` in presented positions; next/previous from **VoiceOver's reading position** (amended 2026-10-08, SP-172 R3: VoiceOver never passes the caret) |
| **AC5** | **Live pass with VoiceOver** (user, Mac): it reads the page without markup at default punctuation verbosity; the rotor moves heading to heading; typing with VoiceOver on behaves |
| **AC6** | **Linux** (Qt accessibility reads the stored text too, I-0277): MEASURE what Orca hears, then rule — a criterion here or EP-048's |

⛔ **OUT:** iOS/visionOS (no manuscript surface yet) · spoken formatting ("bold") beyond what the attributed string carries.

---

## EP-048: `[Linux]` ⚠️ **Manuscript Renderer Parity**

**Status:** 🟡 **ACTIVE** since 2026-10-07 — S1, S2 and SP-173 closed; S3a closed (SP-174, 2026-10-10); S3b closed (SP-175, 2026-10-11); S4 (Find) closed (SP-176, 2026-10-11). L1–L7 and L9 met (S3b, 2026-10-10; Linux has no undo yet and no structured cross-scene paste — recorded in SP-175); L8 and L10 open — ✅ L8's Find half met (S4, [SP-176], rig live pass 2026-10-11); its Replace half is S4b ([SP-181], ✅ closed 2026-10-21). L10 is S5a–S5c: S5a ([SP-182]) ✅ closed 2026-10-21; S5b ([SP-183]) and S5c ([SP-184]) 🔵 Planning. ✅ **EP-048 stays open until EP-026 lands** (user, 2026-10-11).
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
| **[SP-183]** | **S5b** — `[Linux]` L10's block half: line spacing 1.45 and the first-line indent (Qt block formats — measured before the mechanism is ruled) | 🔵 **PLANNING 2026-10-15** → [`../Sprints/Sprint-SP-183.md`](../Sprints/Sprint-SP-183.md) · ⏳ Q1–Q2 after its measurement |
| **[SP-184]** | **S5c** — `[Linux]` the **"Show chapter titles" toggle** ([EP-026] Q4) and the chapter-ending divider ([T-0575]) | 🔵 **PLANNING 2026-10-15** → [`../Sprints/Sprint-SP-184.md`](../Sprints/Sprint-SP-184.md) · ✅ Q1–Q2 ruled 2026-10-15 |

**Tasks:** ✅ [T-0594] (SP-165 — VERIFIED, archived) · ✅ [T-0595] (SP-166 — VERIFIED, archived) · ✅ [T-0601] (SP-174 — VERIFIED, archived) · ✅ [T-0602] (SP-175 — VERIFIED, archived) · ✅ [T-0603] (SP-176 — VERIFIED, archived) · ✅ [T-0608] (SP-181 — VERIFIED 2026-10-21, archived) · ✅ [T-0609] (SP-182 — VERIFIED 2026-10-21, archived) · 🔵 [T-0610] (SP-183) · 🔵 [T-0611] (SP-184).
**Issues:** ✅ [I-0285] (SP-173 — VERIFIED, archived) · 🟠 [I-0280] (found + fixed in SP-165, Apple and core) · 🔵 [I-0281] `[Apple]` (found by L2 → Issue backlog, ➡️ linked forward to [EP-047]).
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

---

---

## EP-026: `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste

**Status:** 🟡 **ACTIVE 2026-10-11** (user: *"Activate EP-026."*) — S1 closed ([SP-177], 2026-10-11): the capture rules are ScriviCore's; S2 ([SP-178]) and S4 ([SP-180]) closed 2026-10-21; S5 closed ([SP-179], 2026-10-15); [SP-185] closed 2026-10-22 (responsive after load). **RESCOPED 2026-10-11** (user: *"We need to implement undo first on Linux. This is a large activity that
is (or should be) planned in a subsequent epic given that Apple has the branching undo history and multiple copy buffers already
in place."* · *"we need to rescope EP-026 and check other epics to see if the copy buffers were mentioned anywhere else."*).
✅ **Q1–Q5 RULED 2026-10-11.** ✅ **[EP-048] stays open until this Epic lands** (user). ⛔ **[EP-048] L8 (Find/Replace, [SP-176]) waits on it** (SP-176 Q3).
**Codebase:** `[Linux]` over a `[ScriviCore]` engine that already exists (`scrivi_history_*` ×15, `scrivi_buffers_*` ×4,
`scrivi_fragment_*` ×4); a `[Cross]` Sprint first if Q1 moves Apple's capture rules into the core.
**Goal:** ✅ **Apple's undo, copy buffers and structured cut/copy/paste on Linux** — the Linux counterpart of **[EP-019]**
(undo/redo history + copy buffers, closed 2026-08-11) AND **[EP-029]** (cross-boundary structured cut/copy/paste, its
structured buffers and its reversible structural undo, closed 2026-08-03). Same history file, same buffers file, so a project
undoes the same on both platforms.

#### What the 2026-10-11 rescope found

| # | Finding | Where |
| - | ------- | ----- |
| 1 | ⛔ **The old EP-026 omitted the copy buffers** — though EP-019 named EP-026 as its Linux side and buffers are EP-019's AC6 | the old entry; `Closed/Epic-EP-019.md` (Codebase line); `Epic-Documentation.md` 2026-07-24 note (*"lock the copy-buffer implementation on Apple before the Linux side (EP-026)"*) |
| 2 | ⛔ **EP-029's Linux follow-on was promised and never filed** (*"Linux/Windows wiring are follow-on Epics"*). Linux BRIDGES the four `scrivi_fragment_*` endpoints (SP-121) and **calls none** — a cross-scene cut on Linux is flat text, no structure ([SP-175] records it) | `Closed/Epic-EP-029.md` Codebase line; `ScriviBridge.cpp:1189–1238`; no other caller |
| 3 | ⚠️ **Stale pointers:** EP-035's out-of-scope table and the ABI audit §6 send the 19 history/buffer endpoints to **EP-019** (closed) with *"Linux story still unruled"* | this file, EP-035 §6 (now pointed here); `Scrivi_ABI_Binding_Gap_Audit_v0_1.md` §6 (a dated audit — not edited) |
| 4 | ⚠️ **"Settings" overlaps [EP-048] L10:** Apple's ONE Project Settings sheet holds Project (title, subtitle) · Writing Surface (show chapter titles — deferred to EP-026 by [EP-023]) · Typography ([EP-047] = EP-048 **L10**) · Undo History (capacity, stale-after days). Linux's Project Settings is a *"coming soon"* stub | `ProjectSettingsSheet.swift`; `ScriviWindow.cpp:310–318`; `Closed/Epic-EP-023.md` |
| 5 | ✅ **"Menus" is OBE** — Linux has had a full menu bar since [EP-043]; the Edit/Format/Find items each arrive with their Epic | `ScriviWindow.cpp` |
| 6 | ⚠️ **"Parity verification + alpha-tester hand-off" is OBE as written** — superseded by the per-Sprint rig live pass (EP-035 §3 ruling) | — |
| 7 | ⚠️ **Apple's capture rules are SWIFT LOGIC** — `HistoryCapture.swift` (590 lines): commit triggers (sentence end, Return, caret move, paste/cut, scene switch, close), 45 s idle sessions, history-before-disk (§4.d), grouped edits, barriers. A Linux port would be a second copy (→ Q1) | `Scrivi/App/HistoryCapture.swift` |
| 8 | ⚠️ **Linux already shaped for it:** every gesture is ONE `QTextCursor` edit block ([EP-049]); `setUndoRedoEnabled(false)` keeps Ctrl+Z free ([EP-022]) | `ManuscriptEditor.cpp`, `EditorShell.cpp` |

#### What Linux must match (Apple, verified)

| Apple | Source | Linux |
| ----- | ------ | ----- |
| Undo/redo in the manuscript, event by event; events per the amended AC2 (no event for caret moves or autosave) | EP-019 AC1, AC2 | ❌ |
| History persists across quit; undo past a session boundary warns once | EP-019 AC3 | ❌ |
| Undo-then-type branches; the fork popover selects a branch; abandoned text restorable | EP-019 AC4; `ForkPopover.swift` | ❌ |
| Capacity setting; stale branches listed and purged (Project ▸ Purge Stale History Branches…) | EP-019 AC5; `StaleBranchesSheet.swift` | ❌ |
| The History card in the Scene Inspector | `Inspector/HistoryCard.swift` | ❌ |
| Copy buffers 1–9: copy / paste / cut chords, each paste one undo step, system clipboard untouched, persist | EP-019 AC6; `BufferService.swift` | ❌ — keys per Q3 |
| ✅ **View ▸ Show Buffers** (Apple ⌥⌘B → Linux **Ctrl+Alt+B** — ✅ RULED 2026-10-11 (user: *"Ctrl-Alt-B is correct."*); unused in the Linux app today; measured against GNOME on the rig) — ✅ required (user, 2026-10-11: *"we need to add it in"*) — a toggle opening the Buffers palette: ONE app-wide floating palette that follows the FRONT project window; each slot's button copies / pastes / cuts by the modifier held (Apple none / ⌃ / ⌥ → Linux none / Ctrl+Shift / Alt, as Q3); disabled with no project open | `ScriviApp.swift:435`; `BuffersPalette.swift`; `AppEnvironment.syncBuffersPalette` | ❌ — ⛔ the old EP-026 never named the menu item |
| Structural ops record barriers (split, merge, delete) — undo stops with a notice | EP-019 AC7 | ❌ |
| Cross-boundary cut/copy/paste carries structure (`scrivi.fragment.v1`): cut merges, paste splits, buffers hold fragments | EP-029 AC1–AC5 | ❌ (bridged, uncalled) |
| Undo of a structured cut/paste restores text AND structure | EP-029 AC6 | ❌ |
| Grouped edits: Replace All and the cross-scene delete are ONE undo step | [SP-164] Q2, [I-0270] | ❌ |

#### Questions — ✅ ALL RULED 2026-10-11

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | Apple's capture rules (finding 7): port them to Linux, or move them into ScriviCore first and re-point Apple ([SP-174]'s pattern)? | ✅ **RULED (*"agreed"*): into the core first** — a `[Cross]` Sprint, Apple re-pointed with no visible change, then Linux wires to it. CLAUDE.md: no backend logic in Swift; a port is a second copy to drift |
| **Q2** | Include EP-029's Linux follow-on (finding 2)? | ✅ **RULED: yes** — structured buffers and structural undo depend on it, and it is the same editing surface |
| **Q3** | Linux keys. Apple: ⌘Z / ⇧⌘Z; buffers ⌘1–9 copy · ⌃1–9 paste · ⌥1–9 cut. ⚠️ Ctrl+Alt+1–3 are Format ▸ Heading 1–3 ([SP-175]), and Linux has no separate ⌃ | ✅ **RULED:** undo **Ctrl+Z** · redo **Ctrl+Shift+Z** (⛔ no Ctrl+Y) · buffers: **Ctrl+1–9 copy into** · **Ctrl+Shift+1–9 paste from** · **Alt+1–9 cut into**. ✅ Checked against Apple's code (`ManuscriptTextView.swift:3295`): ⌘N copy · ⌃N paste · ⌥N cut — so ⌘→Ctrl, ⌥→Alt, and Ctrl+Shift stands in for ⌃, which Linux lacks. ⚠️ Each chord measured against GNOME on the rig; ⚠️ Ctrl+Shift+digit arrives as the SHIFTED key (`!`, `@`… on a US layout) — match the physical digit, proven through the real dispatch |
| **Q4** | Settings (finding 4): who builds Linux's Project Settings dialog? | ✅ **RULED: [EP-048] S5 builds it** (it must, for L10's typography) **with the chapter-titles toggle** (same sheet, deferred since [EP-023]); **EP-026 adds the Undo History section** |
| **Q5** | Drop "menus" and "parity verification / alpha hand-off" (findings 5, 6) | ✅ **RULED: dropped** — OBE |

**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-177]** | **S1** — `[Cross]` the capture rules (`HistoryCapture.swift`) move into ScriviCore; Apple re-pointed, no visible change (Q1) | ✅ **CLOSED 2026-10-11** (user-approved) → [`../Sprints/Closed/Sprint-SP-177.md`](../Sprints/Closed/Sprint-SP-177.md) · ✅ Q1–Q4 ruled · ✅ Mac live pass; [T-0604] verified |
| **[SP-178]** | **S2** — `[Linux]` undo/redo (Ctrl+Z / Ctrl+Shift+Z), persistence, session warning, barriers, grouped edits — ➡️ unblocks [EP-048] S4b | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-178.md`](../Sprints/Closed/Sprint-SP-178.md) · ✅ Q1–Q4 ruled |
| S3 | `[Linux]` branching: fork popover, History card, stale-branch purge, Undo History settings section — ⚠️ the section goes into the dialog [EP-048] S5a ([SP-182]) builds. ➕ 2026-10-24 (user): the chooser as Apple has it now — on Ctrl+Shift+Z only, never after an undo ([I-0320]), branches with no text folded ([I-0321], in the core) — and the branch-point margin marker ([T-0620]: shown with the formatting markers, click opens the chooser) | not yet created |
| **[SP-180]** | **S4** — `[Linux]` structured cut/copy/paste across scenes (EP-029 on Linux); replaces [SP-178] Q2's and [SP-179] Q1's stopgaps | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-180.md`](../Sprints/Closed/Sprint-SP-180.md) · ✅ Q1–Q4 ruled · ✅ AC0–AC9 met (rig 2026-10-21) |
| **[SP-179]** | **S5** — `[Linux]` copy buffers (Ctrl / Ctrl+Shift / Alt + 1–9), View ▸ Show Buffers (Ctrl+Alt+B) and the palette; ⚠️ structured buffers wait for S4 (SP-179 Q1) | ✅ **CLOSED 2026-10-15** (user-approved) → [`../Sprints/Closed/Sprint-SP-179.md`](../Sprints/Closed/Sprint-SP-179.md) · ✅ Q1–Q4 ruled · ✅ rig live pass 2026-10-13; [T-0606] verified · ⚠️ **planned BEFORE S3/S4** on my recommendation (user: *"start planning S5"*) |
| **[SP-185]** | `[Linux]` responsive after load: the 2026-10-15 rig pass's freezes ([I-0307], [I-0297]); [I-0309]; a diagnostic log ([T-0612]) | ✅ **CLOSED 2026-10-22** (user-approved) → [`../Sprints/Closed/Sprint-SP-185.md`](../Sprints/Closed/Sprint-SP-185.md) · [T-0612], [T-0613] verified · AC5 → [EP-052] |

**Tasks:** ✅ [T-0604] (SP-177 — VERIFIED, archived) · ✅ [T-0605] (SP-178 — VERIFIED 2026-10-21, archived; incl. [I-0287]) · ✅ [T-0607] (SP-180 — VERIFIED 2026-10-21, archived; incl. [I-0293]–[I-0296]) · ✅ [T-0606] (SP-179 — VERIFIED, archived; incl. `[Apple]` [I-0289] ✅ verified (balanced buffers) and [I-0290] ✅ verified (paste chord ⌃⌘1–9); [I-0291] ✅ verified (palette modifier)). **Issues found in S2 and ruled into [SP-178]:** ✅ [I-0287] (undo after split/merge — VERIFIED 2026-10-15, archived) · ✅ [I-0288] (undo after an outside edit, in a later session — VERIFIED 2026-10-12, archived). **[SP-185]:** ✅ [T-0612] (VERIFIED 2026-10-22, archived) · ✅ [T-0613] (VERIFIED 2026-10-21, archived) · ✅ [I-0307], [I-0309], [I-0310] (VERIFIED, archived).

**Provisional Sprints (original note):** S1 `[Cross]` capture rules into the core (Q1), Apple re-pointed with no visible change · S2 undo/redo + persistence +
session warning + barriers + grouped edits · S3 branching: fork popover, History card, stale-branch purge, Undo History
settings · S4 structured cut/copy/paste (EP-029 on Linux) · S5 copy buffers, **View ▸ Show Buffers (Ctrl+Alt+B)** and the palette, structured buffers. Each with a rig live
pass. ➡️ After S2, [EP-048] S4b (Replace / Replace All) can run — it needs grouped undo. ⛔ **Not in scope:** Windows; any change to `scrivi.buffers.v1`. ⚠️ **`scrivi.history.v1` was changed TWICE in S2, each ruled in by the user** ([SP-178]): [I-0287] added the optional `barrier.sceneTexts`; [I-0288] made the `externalChange` repair use it, and reads a pre-fix `floor` record (`reason: externalChange`) as its barrier's text. Both backward-compatible: older logs read as before. Any further change is surfaced, not slipped in.
**Depends on:** nothing open — EP-022 (closed), the history/buffer/fragment endpoints (shipped and tested in the core).
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` · `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`.
