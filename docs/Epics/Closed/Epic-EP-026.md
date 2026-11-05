# EP-026: `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste

**Status:** ✅ **CLOSED 2026-11-05** (user, 2026-11-05: *"F1-F3 as recommended.  Close EP-026."*) — ✅ every row of *What Linux must match* met; Sprints [SP-177]–[SP-180], [SP-185] and [SP-204] closed; ✅ Audit Check run at the close (findings below). — activated 2026-10-11 (user: *"Activate EP-026."*) — S1 closed ([SP-177], 2026-10-11): the capture rules are ScriviCore's; S2 ([SP-178]) and S4 ([SP-180]) closed 2026-10-21; S5 closed ([SP-179], 2026-10-15); [SP-185] closed 2026-10-22 (responsive after load); S3 closed ([SP-204], 2026-11-05). **RESCOPED 2026-10-11** (user: *"We need to implement undo first on Linux. This is a large activity that
is (or should be) planned in a subsequent epic given that Apple has the branching undo history and multiple copy buffers already
in place."* · *"we need to rescope EP-026 and check other epics to see if the copy buffers were mentioned anywhere else."*).
✅ **Q1–Q5 RULED 2026-10-11.** ✅ **[EP-048] stays open until this Epic lands** (user).
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
| Undo/redo in the manuscript, event by event; events per the amended AC2 (no event for caret moves or autosave) | EP-019 AC1, AC2 | ✅ [SP-178] AC1, AC2 (rig 2026-10-13) |
| History persists across quit; undo past a session boundary warns once | EP-019 AC3 | ✅ [SP-178] AC3; a Mac history undoes on Linux, AC6 (rig 2026-10-21) |
| Undo-then-type branches; the fork popover selects a branch; abandoned text restorable | EP-019 AC4; `ForkPopover.swift` | ✅ [SP-204] AC1–AC3: the chooser on Ctrl+Shift+Z ([I-0320]) and the margin mark ([T-0620]) (VM 2026-11-05) |
| Capacity setting; stale branches listed and purged (Project ▸ Purge Stale History Branches…) | EP-019 AC5; `StaleBranchesSheet.swift` | ✅ [SP-204] AC5, AC6 (VM 2026-11-05) |
| The History card in the Scene Inspector | `Inspector/HistoryCard.swift` | ✅ [SP-204] AC4: the Writing tab's History section (VM 2026-11-05) |
| Copy buffers 1–9: copy / paste / cut chords, each paste one undo step, system clipboard untouched, persist | EP-019 AC6; `BufferService.swift` | ✅ [SP-179] AC1–AC4, keys per Q3 (rig 2026-10-13) |
| ✅ **View ▸ Show Buffers** (Apple ⌥⌘B → Linux **Ctrl+Alt+B** — ✅ RULED 2026-10-11 (user: *"Ctrl-Alt-B is correct."*); unused in the Linux app today; measured against GNOME on the rig) — ✅ required (user, 2026-10-11: *"we need to add it in"*) — a toggle opening the Buffers palette: ONE app-wide floating palette that follows the FRONT project window; each slot's button copies / pastes / cuts by the modifier held (Apple none / ⌃ / ⌥ → Linux none / Ctrl+Shift / Alt, as Q3); disabled with no project open | `ScriviApp.swift:435`; `BuffersPalette.swift`; `AppEnvironment.syncBuffersPalette` | ✅ [SP-179] AC5 (rig 2026-10-13) |
| Structural ops record barriers (split, merge, delete) — undo stops with a notice | EP-019 AC7 | ✅ [SP-178] AC4 (rig 2026-10-13) |
| Cross-boundary cut/copy/paste carries structure (`scrivi.fragment.v1`): cut merges, paste splits, buffers hold fragments | EP-029 AC1–AC5 | ✅ [SP-180] AC1–AC2, AC4–AC6; buffers hold fragments, AC6 (rig 2026-10-21) |
| Undo of a structured cut/paste restores text AND structure | EP-029 AC6 | ✅ [SP-180] AC3, AC7 (rig 2026-10-21) |
| Grouped edits: Replace All and the cross-scene delete are ONE undo step | [SP-164] Q2, [I-0270] | ✅ the cross-scene delete: [SP-178] AC5 (rig 2026-10-13); Replace All: [EP-048] S4b, [SP-181] AC2 |

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
| **[SP-177]** | **S1** — `[Cross]` the capture rules (`HistoryCapture.swift`) move into ScriviCore; Apple re-pointed, no visible change (Q1) | ✅ **CLOSED 2026-10-11** (user-approved) → [`../../Sprints/Closed/Sprint-SP-177.md`](../../Sprints/Closed/Sprint-SP-177.md) · ✅ Q1–Q4 ruled · ✅ Mac live pass; [T-0604] verified |
| **[SP-178]** | **S2** — `[Linux]` undo/redo (Ctrl+Z / Ctrl+Shift+Z), persistence, session warning, barriers, grouped edits — ➡️ unblocks [EP-048] S4b | ✅ **CLOSED 2026-10-21** (user-approved) → [`../../Sprints/Closed/Sprint-SP-178.md`](../../Sprints/Closed/Sprint-SP-178.md) · ✅ Q1–Q4 ruled |
| S3 | `[Linux]` branching: fork popover, History card, stale-branch purge, Undo History settings section — ⚠️ the section goes into the dialog [EP-048] S5a ([SP-182]) builds. ➕ 2026-10-24 (user): the chooser as Apple has it now — on Ctrl+Shift+Z only, never after an undo ([I-0320]), branches with no text folded ([I-0321], in the core) — and the branch-point margin marker ([T-0620]: shown with the formatting markers, click opens the chooser) | ✅ **CLOSED 2026-11-05** → [SP-204] [`../../Sprints/Closed/Sprint-SP-204.md`](../../Sprints/Closed/Sprint-SP-204.md) · [T-0662] ✅ · ➕ [I-0350] ✅ |
| **[SP-180]** | **S4** — `[Linux]` structured cut/copy/paste across scenes (EP-029 on Linux); replaces [SP-178] Q2's and [SP-179] Q1's stopgaps | ✅ **CLOSED 2026-10-21** (user-approved) → [`../../Sprints/Closed/Sprint-SP-180.md`](../../Sprints/Closed/Sprint-SP-180.md) · ✅ Q1–Q4 ruled · ✅ AC0–AC9 met (rig 2026-10-21) |
| **[SP-179]** | **S5** — `[Linux]` copy buffers (Ctrl / Ctrl+Shift / Alt + 1–9), View ▸ Show Buffers (Ctrl+Alt+B) and the palette; ⚠️ structured buffers wait for S4 (SP-179 Q1) | ✅ **CLOSED 2026-10-15** (user-approved) → [`../../Sprints/Closed/Sprint-SP-179.md`](../../Sprints/Closed/Sprint-SP-179.md) · ✅ Q1–Q4 ruled · ✅ rig live pass 2026-10-13; [T-0606] verified · ⚠️ **planned BEFORE S3/S4** on my recommendation (user: *"start planning S5"*) |
| **[SP-185]** | `[Linux]` responsive after load: the 2026-10-15 rig pass's freezes ([I-0307], [I-0297]); [I-0309]; a diagnostic log ([T-0612]) | ✅ **CLOSED 2026-10-22** (user-approved) → [`../../Sprints/Closed/Sprint-SP-185.md`](../../Sprints/Closed/Sprint-SP-185.md) · [T-0612], [T-0613] verified · AC5 → [EP-052] |

**Tasks:** ✅ [T-0604] (SP-177 — VERIFIED, archived) · ✅ [T-0662] (SP-204 — VERIFIED 2026-11-05, archived) · ✅ [T-0605] (SP-178 — VERIFIED 2026-10-21, archived; incl. [I-0287]) · ✅ [T-0607] (SP-180 — VERIFIED 2026-10-21, archived; incl. [I-0293]–[I-0296]) · ✅ [T-0606] (SP-179 — VERIFIED, archived; incl. `[Apple]` [I-0289] ✅ verified (balanced buffers) and [I-0290] ✅ verified (paste chord ⌃⌘1–9); [I-0291] ✅ verified (palette modifier)). **Issues found in S2 and ruled into [SP-178]:** ✅ [I-0287] (undo after split/merge — VERIFIED 2026-10-15, archived) · ✅ [I-0288] (undo after an outside edit, in a later session — VERIFIED 2026-10-12, archived). **[SP-185]:** ✅ [T-0612] (VERIFIED 2026-10-22, archived) · ✅ [T-0613] (VERIFIED 2026-10-21, archived) · ✅ [I-0307], [I-0309], [I-0310] (VERIFIED, archived).

**Provisional Sprints (original note):** S1 `[Cross]` capture rules into the core (Q1), Apple re-pointed with no visible change · S2 undo/redo + persistence +
session warning + barriers + grouped edits · S3 branching: fork popover, History card, stale-branch purge, Undo History
settings · S4 structured cut/copy/paste (EP-029 on Linux) · S5 copy buffers, **View ▸ Show Buffers (Ctrl+Alt+B)** and the palette, structured buffers. Each with a rig live
pass. ➡️ After S2, [EP-048] S4b (Replace / Replace All) can run — it needs grouped undo. ⛔ **Not in scope:** Windows; any change to `scrivi.buffers.v1`. ⚠️ **`scrivi.history.v1` was changed TWICE in S2, each ruled in by the user** ([SP-178]): [I-0287] added the optional `barrier.sceneTexts`; [I-0288] made the `externalChange` repair use it, and reads a pre-fix `floor` record (`reason: externalChange`) as its barrier's text. Both backward-compatible: older logs read as before. Any further change is surfaced, not slipped in.
**Depends on:** nothing open — EP-022 (closed), the history/buffer/fragment endpoints (shipped and tested in the core).
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` · `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`.

### ✅ Audit Check at the close (2026-11-05) — ruled by the user ("F1-F3 as recommended")

| # | Finding | Ruling |
|---|---|---|
| F1 | The Status line listed S1, S2, S4, S5 and [SP-185] as closed but not S3 ([SP-204]) | ✅ Fixed at the close (above) |
| F2 | `Closed/Sprint-SP-178.md` and `Sprint-SP-180.md` read `status: Active` with no `closed:` (their bodies: closed 2026-10-21); the same in [SP-136] (EP-040), [SP-181] and [SP-182] (EP-048) | ✅ Fixed, all five |
| F3 | [I-0270] (the grouped-edits evidence) is archived as a table row only, no `## I-0270` section; likewise I-0266, I-0269, I-0281–I-0283, I-0329 | ✅ Accepted as the older format — the rows hold the detail; nothing changed |

Clean: Epic status (index and `Epic-active.md` agree; 5 → 4 active), evidence (T-0604–T-0607, T-0612, T-0613, T-0620, T-0662 and
17 Issues archived), Sprint status (SP-177–SP-180, SP-185, SP-204 closed everywhere), no orphan files, ID continuity (next EP-058,
SP-205, T-0664, I-0351).
