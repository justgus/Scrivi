---
sprint: SP-179
epic: EP-026
status: Planning
platform: Linux
created: 2026-10-12
---

# SP-179 — `[Linux]` [EP-026] **S5**: copy buffers 1–9, View ▸ Show Buffers (Ctrl+Alt+B) and the Buffers palette

**Status:** 🟡 **ACTIVE 2026-10-12** (user: *"I'll go with Option A. Fix it here."*) — created 2026-10-12 (user: *"fix the Epic record and start planning S5"*). ✅ Q1–Q4 ruled and the plan accepted 2026-10-12. ✅ Step 0 complete: [I-0289], [I-0290], [I-0291] all VERIFIED (Apple). ⏳ Linux steps 1–8 next.
**Carries:** [T-0606] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-026] `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste →
[`../Epics/Epic-active.md`](../Epics/Epic-active.md). ⚠️ **Planned BEFORE S3 and S4** on my recommendation (user: *"start planning S5"*): it depends least on
[SP-178]'s not-yet-live-passed undo path, and its key chords can be measured on the rig in the same session as SP-178's pass.
**Builds on:** [SP-178] (S2) — a cut into a slot and a paste from one are UNDO steps, through S2's `HistoryCapture`.
⚠️ SP-178 is implemented but not live-passed; a defect its pass finds in the undo path may touch this Sprint.
**Authority:** [`../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`](../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md)
§9 (buffers), Trade T3 (copy ≠ event · cut = event · paste = paste event), T4 (palette). EP-019 AC6. EP-026 Q3 (keys, ruled
2026-10-11) and the Show Buffers ruling (*"Ctrl-Alt-B is correct."*).
**Shape:** ✅ Apple's (`feedback_linux_adopts_apple_shape`): `BufferService.swift` (129 lines — the slot mirror, ScriviCore
only), `BuffersPalette.swift` (275 — one floating panel, one modifier-sensitive button per slot), the coordinator's
`copyIntoBuffer` / `cutIntoBuffer` / `pasteFromBuffer` (`ManuscriptTextView.swift:840–921`), the chords in `keyDown`
(`:3285–3307`), and `AppEnvironment.syncBuffersPalette` (`AppEnvironment.swift:115`).
**Size:** M. ⚠️ Needs a **rig live pass** (every chord measured against GNOME over RDP).

---

## What reading the code found (2026-10-12)

✅ **The store is done:** `scrivi_buffers_load` / `_get` / `_list` / `_clear` (`scrivi.h:1029–1046`) — stateless
read-modify-write of `history/buffers.json` (`scrivi.buffers.v1`), slots "1"–"9", no open/close. Linux binds none of them
(`ScriviBridge.hpp:508`: *"`scrivi_buffers_*` (S5)"*).

| Need | Apple | Linux today |
| ---- | ----- | ----------- |
| The four endpoints | `ScriviEngine.buffers*` | ⛔ not bridged |
| Slot mirror (one per open project) | `BufferService`, owned by `ProjectSession` | ⛔ none |
| Copy / cut / paste into the editor | coordinator: `copyIntoBuffer` (`:840`), `cutIntoBuffer` (`:865`), `pasteFromBuffer` (`:900`) | ⛔ none. Its ⌘C/⌘X/⌘V equivalents are `createMimeDataFromSelection` (`ManuscriptEditor.cpp:712`), `cutSelection` (`:850`), `insertFromMimeData` (`:759`) |
| A cut is a `cut` undo step tagged with the slot | `beginCutIntoBuffer(bufferID:)` → `scrivi_capture_begin(kind, bufferID)` | ✅ bridge takes `bufferID` (`ScriviBridge.cpp:2021`); ⚠️ `HistoryCapture::begin` (`HistoryCapture.cpp:121`) drops it — one argument |
| Chords | ⌘1–9 copy · ⌃1–9 paste · ⌥1–9 cut (`:3285–3307`) | ✅ free: no Ctrl+digit, Ctrl+Shift+digit or Alt+digit shortcut in `platforms/linux/src/`. Ctrl+Alt+0–3 are Format ([SP-175]) |
| View ▸ Show Buffers | ⌥⌘B toggle (`ScriviApp.swift:435`) | ⛔ none; Ctrl+Alt+B unused |
| The palette follows the FRONT project | `AppEnvironment.frontmostSession` → `syncBuffersPalette` | ⛔ **Linux has no notion of a front project.** `ProjectWindowManager` maps projectID → window, *"order is NOT significant"* (`ProjectWindowManager.hpp:69`) — new plumbing in `AppEnvironment` |
| A cross-scene copy into a slot stores a FRAGMENT | `fragmentExtract` → structured slot; paste rebuilds scenes (`pasteStructuredFragment`, `:2581`) | ⛔ S4's. Linux bridges `scrivi_fragment_*` and calls none ([SP-175]) → **Q1** |

⚠️ **Three things found by reading:**
1. **Apple's buffers skip the balancing its ⌘C/⌘X/⌘V do.** ⌘C goes through `presenter.balancedCopy` (`writeSelection`,
   `ManuscriptTextView.swift:3014–3023`, EP-046 AC12). Linux's ⌘C is already balanced (`ManuscriptEditor.cpp:712`).
   ⚠️ **Read, not run.** Re-read in full 2026-10-12 (user: *"Is there a gap there?"*) — three gaps, all in what the SLOT holds
   or how it is pasted; the scene text itself always stays balanced (`shouldChangeText`'s `balancedEdit`):
   - **(a) ⌘N / ⌥N, one scene:** the slot gets raw storage (`:855`, `:884`). Selecting `old te` inside `**bold text**` stores
     `old te`, so the paste is NOT bold (⌘C stores `**old te**`). Selecting from before `**` to mid-span stores `**bold te`,
     whose paste is re-balanced only by the paste-side edit.
   - **(b) ⌘N / ⌥N, across scenes:** the slot's fragment comes straight from `fragmentExtract` (`:850`); ⌘C balances each
     scene's piece first (`structuredCopyIfCrossBoundary`, `:2416`, [T-0591]).
   - **(c) ⌃N paste:** inserts verbatim but WITHOUT `pastingOwnCopy` (`:3213`), so the balanced edit takes the CARET's style
     (plain text pasted inside bold turns bold). Scrivi's own ⌘V keeps the copy's own formatting (`:3069`, [SP-163] live-pass
     ruling). A structured slot's paste is already right (`pasteStructuredFragment` balances against the caret, as ⌘V does).
   → **Q2**
2. **`BufferService.swift`'s header comment is stale.** It describes a context-sensitive ⌘N (copy with a selection, paste
   without) and ⇧⌘N cut. The code's own comment at `:3289` says that design was REPLACED (*"the earlier context-sensitive
   ⌘N was ambiguous"*). EP-026 Q3 read the code, not the comment, and is right. → Q2 (Apple-side touch)
3. **Ctrl+Shift chords and the rig.** Linux code records that the macOS→VNC path EATS Ctrl+Shift combos
   (`ScriviWindow.cpp:275`, `project_linux_vnc_input_constraints`). Paste is **Ctrl+Shift+1–9** (Q3 ruling), and SP-178's redo
   is Ctrl+Shift+Z. The rig is now RDP, not VNC. ✅ **SP-178's live pass (AC2) measures Ctrl+Shift+Z over RDP first.** If it
   fails there, this Sprint's paste chord needs the same answer, and the palette's Ctrl+Shift button is the fallback that
   always works. ⚠️ Q3 also noted Ctrl+Shift+digit arrives as the SHIFTED key (`!`, `@`… on a US layout): match the physical
   digit (`nativeScanCode` / `Qt::Key_1…9` after un-shifting), proven through the real dispatch.

## Goal

✅ **Apple's copy buffers on Linux:** Ctrl+1–9 copies the selection into a slot (no undo step), Ctrl+Shift+1–9 pastes a slot
(one undo step), Alt+1–9 cuts into a slot (one undo step, tagged with the slot); the system clipboard is untouched; slots
persist per project and are the SAME file the Mac reads. View ▸ Show Buffers (Ctrl+Alt+B) opens ONE app-wide palette that
follows the front project window, one button per slot whose action follows the held modifier.

## Plan

0. ✅ **`[Apple]` first ([I-0289], Q2 Option A):** the Mac buffers made balanced, measured by failing interop tests first.
1. **Bridge** `scrivi_buffers_load` / `_get` / `_list` / `_clear` (`ScriviBridge`), static and best-effort like S2's (logged,
   never `errorOccurred`). Update the *"deliberately absent"* note. Extend `bridge_parity_smoke` (each endpoint, an
   invalid slot, a reopen).
2. **`BufferService` (Linux)** — `platforms/linux/src/BufferService.{hpp,cpp}`, one per open project, owned by
   `ProjectSession`: the slot mirror (`refresh`, `text(slot)`, `fragment(slot)`, `preview(slot)` = the PRESENTED first line,
   as Apple's), `load`, `clear`; a `changed()` signal for the palette. ⛔ No text mutation — Apple's rule.
3. **Editor actions** (Apple's three, one for one) on `ManuscriptEditor`, routed through `EditorShell`:
   - **copy into slot** — per Q2; a cross-scene selection per Q1. No history event (T3).
   - **cut into slot** — load the slot FIRST, then `begin("cut", slot)` → the removal (`replaceRange`, or
     `deleteAcrossScenes` across scenes, [I-0270]) → `flush("cut")`. One undo step.
   - **paste from slot** — the slot's STORED text inserted verbatim (never re-escaped, EP-045 AC4), replacing the
     selection, inside `begin("paste")` / `flush`. Empty slot → silent no-op. The editable-range gate applies as for any paste.
   - `HistoryCapture::begin` gains the `bufferID` argument.
4. **Chords** — matched in `ManuscriptEditor::keyPressEvent` (and `ShortcutOverride`, as S2's Ctrl+Z), each requiring its
   modifiers and no others: Ctrl+digit copy · Ctrl+Shift+digit paste · Alt+digit cut. Physical digit (trap 3).
5. **Front project** — `AppEnvironment` tracks the front `ScriviWindow` (`QGuiApplication::focusWindowChanged`, ignoring the
   palette itself and the Landing window) and exposes its `ProjectSession`.
6. **View ▸ Show Buffers** (Ctrl+Alt+B, checkable) in every project window, all bound to ONE app-wide visibility flag in
   `AppEnvironment`; disabled with no project open. **The palette** — `platforms/linux/src/BuffersPalette.{hpp,cpp}`: a
   non-modal `Qt::Tool` window that does not take focus from the editor (`WA_ShowWithoutActivating`), re-pointed at the front
   project's `BufferService` when the front window changes, hidden when no project is open. Nine rows: slot number, presented
   preview (*"empty"*), friendly time (`HistoryCapture::friendlyTimestamp`), one action button, a clear button. The button
   follows the held modifier (none → copy · Ctrl+Shift → paste · Alt → cut), tracked by an application event filter
   (the palette never has focus). Closing it with its X unchecks the menu item in every window.
7. **Tests** (offscreen smokes, keys through `QTest` into the real widget, text read FROM DISK): each chord does its one
   thing; copy makes no undo step; a cut and a paste are ONE step each and undo restores text and disk; the cut's history
   node carries the slot; the system clipboard is unchanged; an empty slot pastes nothing; a slot written on Linux reads back
   after reopen and through `scrivi_buffers_list` (the Mac's view); stored text with escapes round-trips verbatim; two
   windows → the palette shows the front one's slots. Mutation-checked. ctest macOS and Linux (Docker, non-root).
8. **Rig live pass** (user), steps in the chat reply, a build check first (`--version`). ⚠️ Every chord measured against GNOME
   over RDP; shares a rig session with [SP-178]'s pass if both are ready.

## Questions — ✅ all ruled 2026-10-12

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Structured slots before S4.** On Apple a cross-scene copy into a slot stores a `scrivi.fragment.v1` fragment and its paste rebuilds the scene breaks. Linux has no structured paste until S4. What do Linux slots do meanwhile — and with a structured slot the MAC wrote (same file)? | ✅ **Plain text until S4, said once:** a cross-scene copy/cut into a slot stores its flat text, no fragment (Linux's ⌘C/⌘X do the same today, [SP-175]). Pasting a Mac-written structured slot pastes its plain `text` and shows a notice: *"This buffer holds scenes copied on the Mac. It is pasted as plain text; scene breaks are not restored yet."* Nothing is lost: the slot keeps its fragment. Alternative: refuse that paste with the notice | ✅ **As recommended** (user, 2026-10-12) |
| **Q2** | **Balanced or raw?** Linux's ⌘C copies BALANCED Markdown (EP-046 AC12); Apple's ⌘N copies RAW storage (finding 1). Which does Linux's slot copy follow? | ✅ **Balanced — the same source text Linux's ⌘C puts on its own clipboard** (`createMimeDataFromSelection`'s `source`), and the cut's removal is the balanced one (`replaceRange`), as `cutSelection` already does. Then **file an `[Apple]` Issue** for ⌘N/⌥N skipping the balancing (measure first, with an interop test), and fix `BufferService.swift`'s stale header comment there. ⚠️ Alternative: mirror Apple's raw copy now and fix both platforms together later | ✅ **Option A** (user, 2026-10-12: *"I'll go with Option A. Fix it here."*) — balanced on BOTH platforms, ⚠️ **Apple fixed FIRST, inside this Sprint** ([I-0289], all three gaps of finding 1 + the stale comment), so Linux mirrors the corrected shape. (Options were A: fix Apple here first · B: Linux balanced, Apple later · C: Linux raw like Apple, both later.) |
| **Q3** | **The palette's window on Wayland.** A Qt `Qt::Tool` window cannot be placed or kept on top reliably under Wayland (EP-043 ruled position restore unachievable there). | ✅ **Accept the compositor's placement**: open it beside the front window where the platform allows, never restore its position, and keep it non-activating. ⛔ Not a docked panel: Apple's is a floating panel, and a dock would be per-window, not app-wide | ✅ **As recommended** (user, 2026-10-12) |
| **Q4** | **Clear a slot from the keyboard?** Apple clears only from the palette's ✕ button. | ✅ **Same as Apple: palette only.** No new chord | ✅ **No — as on Apple** (user, 2026-10-12) |

⛔ **NOT in this Sprint:** structured (fragment) slots and structured paste (S4) · the fork popover, History card, Undo History
settings (S3) · Replace / Replace All ([EP-048] S4b) · Windows · any change to ScriviCore or `scrivi.buffers.v1` · ✅ the Apple
change Q2 ruled ([I-0289]: the three buffer gaps + the stale comment) — ⛔ no other.

## Acceptance Criteria

- [x] AC0 — `[Apple]` [I-0289]: ⌘N / ⌥N store the balanced copy (one scene and across scenes); ⌃N keeps the slot's own formatting; `BufferService.swift` header current. `BufferBalanceTests` 4/4, each failing before the fix; interop 281/281. ✅ **VERIFIED 2026-10-12** (user, Mac)
- [x] AC0b — `[Apple]` [I-0290]: buffer paste is ⌃⌘1–9 (⌃1–9 is macOS's "Switch to Desktop N"); menu and palette labels follow. `BufferPasteChordTests` 2/2, mutation-checked; interop 283/283. ✅ **VERIFIED 2026-10-12** (user, Mac)
- [x] AC0c — `[Apple]` [I-0291]: the palette's button turns to paste on ⌃⌘, not ⌃ alone. Test failed before the fix; interop 284/284. ✅ **VERIFIED 2026-10-12** (user, Mac)
- [ ] AC1 — Ctrl+1–9 copies the selection into slot N: no text change, no undo step, system clipboard untouched; nothing selected → no-op
- [ ] AC2 — Ctrl+Shift+1–9 pastes slot N at the caret, replacing any selection: ONE undo step, stored text verbatim; an empty slot → no-op
- [ ] AC3 — Alt+1–9 cuts the selection into slot N: ONE undo step, its history node tagged with the slot; undo restores text and disk, and the slot keeps the text
- [ ] AC4 — Slots persist per project across quit, in the same `history/buffers.json` the Mac reads and writes (a Mac-written slot pastes on Linux per Q1)
- [ ] AC5 — View ▸ Show Buffers (Ctrl+Alt+B) toggles ONE app-wide palette that follows the front project window: nine rows, presented preview, time, a modifier-following action button, clear; disabled with no project open; closing it unchecks the menu item
- [ ] AC6 — Copy into a slot is balanced or raw per Q2; a cross-scene copy/cut per Q1
- [ ] AC7 — Smokes green through the real key dispatch, mutation-checked; `bridge_parity_smoke` covers the four endpoints; ctest green on macOS and Linux
- [ ] AC8 — ✅ **Rig live pass (user)** — every chord measured against GNOME over RDP

---

## Progress log

### 🔵 2026-10-12 — Sprint CREATED in Planning

✅ User: *"fix the Epic record and start planning S5"* (S5 before S3/S4, on my recommendation: least dependent on SP-178's
unverified undo path; its chords share SP-178's rig session). ✅ SP-179 and [T-0606] issued by `next-id.py`. ✅ Read first:
Apple's `BufferService.swift`, `BuffersPalette.swift`, the coordinator's buffer actions and `keyDown` chords,
`AppEnvironment.syncBuffersPalette`; the `scrivi_buffers_*` ABI; Linux's copy/cut/paste, shortcuts, window registry and
capture bridge. Three findings (Apple's unbalanced buffer copy, a stale Apple comment, Ctrl+Shift over the rig). Q1–Q4
await ruling.

### 2026-10-12 — Q1, Q3, Q4 ruled; plan accepted; Q2 held

✅ User: *"Q1: your recommendation. Q3: your recommendation. Q4: no, as on Apple. … Other than that I accept your plan."*
⏳ Q2 not ruled; user asked: *"Do we need to look at balanced cut/copy/paste buffers on Apple? Is there a gap there?"* Apple's
buffer paths re-read in full: three gaps (finding 1 a–c). ⚠️ Read, not run. Still in Planning; activation on the user's word.

### 🟡 2026-10-12 — ACTIVATED; step 0 ([I-0289], Apple) done

✅ User: *"So Q2 is the balance gap on Apple? I'll go with Option A. Fix it here."* Q2 = Option A; SP-179 activated; [T-0606]
→ active. ✅ [I-0289] filed `[Apple]` (no earlier record: `Issues/`, `Tasks/`, `Closed/`, `Verified/` searched).
✅ **Measured before fixing:** `BufferBalanceTests` (4) written first — all 4 FAILED on the old code (a: ⌘N, a: ⌥N, b, c); the
scene-stays-balanced expectation passed, as the read said. ✅ **Fixed** in `ManuscriptTextView.swift` (`balancedSource`,
`balancedFragment` shared with ⌘C, `pastingOwnCopy` in `insertTextForBuffer`) and `BufferService.swift`'s header. ✅ 4/4, interop
281/281. ⏳ Next: Linux, plan steps 1–8.

### 2026-10-12 — [I-0289] Mac check: steps 2 and 4 failed (user); diagnosed, not fixed

User: *"both 2 and 4 failed. in 2 Ctrl-1 did not paste at all. In 4 Ctrl-2 pasted, but it pasted bold in the bold section."*
Running build confirmed current (15:02, after the 14:55 edit; `balancedSource`/`balancedFragment` in `Scrivi.debug.dylib`).
- **Step 2:** `[SCRIVI-EDIT] keyDown` logged ONE "1" key (15:06:25, the ⌘1 copy; `buffers.json` slot 1 = the balanced
  `ng ***one*** does not choose…`). ⛔ **⌃1 never reached Scrivi:** macOS Mission Control *Switch to Desktop 1–9* is bound to
  ⌃1–⌃9 and enabled on this Mac (`com.apple.symbolichotkeys` 118–126). ⌃2 DID reach Scrivi, so (inferred, not measured) macOS takes ⌃N only when desktop N exists — ⌃1 is always
  taken. A chord collision in the Apple design (SP-056), not this fix; not filed before. ⏳ Ruling owed.
- **Step 4:** `history/log-000001.jsonl` seq 6034: ⌃2 pasted `stains it. He wr` at byte 426 of `## The King ***one*** does |not
  choose` — a HEADING line, after "does ", OUTSIDE the `***one***` span; the core rightly added no markers. Heading text displays
  bold. ⏳ Retest in body text (e.g. `seven **stars** league`).

### 2026-10-12 — [I-0289] VERIFIED; [I-0290] filed and fixed (Option B)

✅ User: *"Option B, fix it in SP-179. Oh and the operator corrections "fixed" the problems with the paste in 2 and 4."* Steps 2
and 4 passed when re-run (slot 2; body-text bold) → [I-0289] VERIFIED and archived. ✅ [I-0290] `[Apple]` filed: buffer paste
⌃1–9 → ⌃⌘1–9, checked free in macOS's symbolic hotkeys and Scrivi's menus first. `BufferPasteChordTests` 2/2 (mutation: the old
⌃-only branch FAILS it); interop 283/283. Linux's paste chord (Ctrl+Shift+1–9, EP-026 Q3) is unaffected.

### 2026-10-12 — [I-0290] VERIFIED; [I-0291] filed and fixed

✅ User: *"I-0290 passes. But the view buffers icon changes on holding Ctrl down, not Ctrl-Cmd."* → [I-0290] VERIFIED, archived.
[I-0291] `[Apple]`: I had left the palette's button on ⌃ alone; it now follows ⌃⌘, as the chord does. Test written first and
FAILED on the old code; interop 284/284.

### 2026-10-12 — [I-0291] VERIFIED; step 0 (Apple) complete

✅ User: *"I-0291 passes"* (incl. the ⌃⌘-click paste) → archived to the new batch `Issue-verified-0291-0300.md`. All three
Apple Issues of step 0 are verified. ⏳ Next: Linux, plan steps 1–8.
