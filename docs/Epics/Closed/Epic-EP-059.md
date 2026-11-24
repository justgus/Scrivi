## EP-059: `[Linux]` **Linux catches up with the Mac** — every capability gap, one Epic

**Status:** ✅ **CLOSED 2026-11-23** (user, 2026-11-23: *"Fix F1 and F2, then close EP-059"*) — every AC met (A1–A9, B1–B6, C1–C6, D1–D3, E1–E3, F1); Sprints [SP-230]–[SP-240] closed. Audit Check (2026-11-23): F1 (this Epic's index row malformed) and F2 (stale pending text in closed SP-237–SP-240) fixed at close; F3–F5 (the Issue total, batch 15's count, the Task statistics) pre-existing, a full Audit recommended. Was 🟡 **ACTIVE** 2026-11-20 (user, 2026-11-20: *"Nope.  Can't archive yet.  We need to do the Linux gap epic first."*); part E = [SP-230] ✅ closed 2026-11-21; part D = [SP-231] ✅ closed 2026-11-21; part F (toolbar) = [SP-232] ✅ closed 2026-11-21; part A's first Sprint = [SP-233] ✅ closed 2026-11-22; its second = [SP-234] ✅ closed 2026-11-22; part B (B1–B4) = [SP-235] ✅ closed 2026-11-22; part C (C1–C4) = [SP-236] ✅ closed 2026-11-22; C5/C6 = [SP-237] (`[Apple]`) ✅ closed 2026-11-23 and [SP-238] (Linux) ✅ closed 2026-11-23; A8 = [SP-239] (`[Apple]`) ✅ closed 2026-11-23; A9 = [SP-240] (Linux) ✅ closed 2026-11-23. Created 2026-11-20 (user, 2026-11-20: *"yes, file G1–G9 as an Epic (include everything here, so it is all tracked at once."*). ✅ It **supersedes [EP-035], [EP-036] and [EP-037]**
(user, 2026-11-20: *"One Epic for all"*), which are closed into it: their ACs are carried below by reference, verified ones kept.
**Source:** [`../Scrivi_Linux_Capability_Gap_Review_v0_1.md`](../../Scrivi_Linux_Capability_Gap_Review_v0_1.md) (G1–G9, read from the code
2026-11-20). The core is at parity; every gap is a Linux surface.
**Codebase:** `[Linux]` — Qt Widgets over `ScriviBridge`. ⚠️ **Linux adopts Apple's shape** (the Mac is the reference); where
Linux differs because that is right on Linux, it is not a gap.
**Goal:** a writer on Linux can do everything a writer on the Mac can: work with the world's objects (open, edit, create, link,
relate, cite), manage worlds (create one, reconnect), see when a world is away, find and navigate scenes, and reach the verbs from a toolbar, as
on the Mac.

**Acceptance Criteria:**

*A. Objects — from [EP-035] (Object Foundations; its record: [`Closed/Epic-EP-035.md`](Epic-EP-035.md)):*
- [x] **A1** (EP-035 AC3) — ✅ world binding from the app: add existing, status, Locate…, remove (SP-127, 2026-09-02)
- [x] **A2** (EP-035 AC10) — ✅ the Scene Inspector's three tabs, Writing | Worldbuilding | Properties (SP-126, 2026-08-30)
- [x] **A3** ✅ [SP-234] (EP-035 AC1) — open an object by double-click and by menu, in the Scene Inspector; the project-wide list is the Add <Kind>… picker, as the Mac's ([SP-234] Q1)
- [x] **A4** ✅ [SP-234] (EP-035 AC2) — the objects listed by kind, kind scope derived: the scene's, and the project's in the Add <Kind>… picker; no separate browser, as the Mac ([SP-234] Q1)
- [x] **A5** ✅ [SP-234] (EP-035 AC4; **G2**, [T-0696]) — object create, open, save, delete, promote, with `worldID` threaded through every call; an
      object linked to and removed from a scene from its card (Apple: `ObjectCard.swift` Add existing… / New / Remove from Scene); ⚠️ delete and promote have no surface on the Mac either → A8, A9 (2026-11-21)
- [x] **A6** ✅ [SP-234] (EP-035 AC5, [T-0696]) — a card's list item shows the object's thumbnail when it has an image
- [x] **A7** ✅ [SP-233] (**G4**, [T-0491]) — a stack's sort (manual / name / recently added) and card reorder → [SP-233] (⚠️ the Mac has no
      reorder — `moveCards` has no caller; [SP-233] Q2)
- [x] **A8** `[Apple]` — Delete Object and Promote on the Mac and iOS, which have neither today (the core has both). New 2026-11-21
      (user, 2026-11-21: *"Build them on Linux and Mac and iOS (will require two Sprints.  One for Linux and one for MacOS/iOS).  The
      Linux sprint should block on the MacOS/iOS sprint's completion."*) — [T-0702] → [SP-239] ✅
- [x] **A9** — Delete Object and Promote on Linux, in the shape A8 sets. ✅ Unblocked ([SP-239] closed). [T-0703] → [SP-240] ✅

*B. The detail sheet and media — from [EP-036] ([`Closed/Epic-EP-036.md`](Epic-EP-036.md)):*
- [x] **B1** (EP-036 AC1; **G1**, [T-0697] → [SP-235]) — the object detail view, in place of today's "not built yet" message (`EditorShell.cpp:4503`):
      views and edits name, subtitle, notes, tags; edits persist
- [x] **B2** (EP-036 AC2, [T-0697] → [SP-235]) — an image imported, shown, replaced, removed
- [x] **B3** (EP-036 AC3, [T-0697] → [SP-235]) — a pending object opens read-only, explained, never written
- [x] **B4** (EP-036 AC4, [T-0697] → [SP-235]) — a world going away while the sheet is open loses no data
- [x] **B5** ✅ [SP-233] (EP-036 AC4a; **G4**) — the writer's card stack is honoured, read from `inspector-layout.json` → [SP-233], brought
      forward: A5's verbs live on a kind's card, and Linux has none
- [x] **B6** ✅ [SP-233] (EP-036 AC4b; **G4**) — the stack picker: add and remove cards on a stack (depends on B5) → [SP-233]

*C. Relationships and sources — from [EP-037] ([`Closed/Epic-EP-037.md`](Epic-EP-037.md)):*
- [x] **C1** (EP-037 AC1, [T-0698] → [SP-236]) — the related-objects section, labels read from the right endpoint
- [x] **C2** (EP-037 AC2, [T-0698] → [SP-236]) — a relationship created from the sheet, shown from both ends
- [x] **C3** (EP-037 AC3, [T-0698] → [SP-236]) — push-navigation to a related object, with Back (✅ no Forward, as the Mac — user ruling 2026-09-23; [SP-236] Q2)
- [x] **C4** (EP-037 AC4; **G3**, [T-0699] → [SP-236]) — sources created from the object they document, with citation detail
      (shown, not edited — the Mac has no editor, [I-0370]; footnote text is [EP-032]'s; [SP-236] Q4); a scene shows the sources its
      objects cite (Apple: `SourcesCard.swift`)
- [x] **C5** (EP-037 AC5, [T-0700] → [SP-237] the Mac and iOS ✅, then [T-0472] → [SP-238] Linux ✅) — a surface for custom relation types (`scrivi_upsert_relation_type`)
- [x] **C6** (EP-037 AC6, [T-0700] → [SP-237] ✅) — a C ABI test for `scrivi_upsert_relation_type`

*D. Worlds — new 2026-11-20:*
- [x] **D1** ✅ [SP-231] (**G5**, [T-0497]; [SP-231]) — a world can be created from Manage Worlds…, so a Linux-only writer with no world can proceed
- [x] **D2** ✅ [SP-231] (**G5**, [T-0693]; [SP-231]) — Reconnect Worlds, as the Mac's (`WorldsView.swift`)
- [x] **D3** ✅ [SP-231] (**G6**, [T-0693]; [SP-231]) — the world-warning strip (View ▸ Show World Warnings): the project-wide notice when a world is away, with
      Manage Worlds…; an imported timeline that failed to read is reported, not only logged — ✅ in the Timeline panel's own banner, as
      the Mac's (`TimelineStripView.swift:950`), not in the world strip ([SP-231] Q4, 2026-11-21)

*E. Manuscript and navigation — new 2026-11-20:*
- [x] **E1** ✅ [SP-230] (**G7**) — an untitled scene shows its first line as the manuscript draws it, or "Scene N" when it is empty, in the
      Navigator and everywhere else a scene is named (today "Untitled Scene", `EditorShell.cpp:2063`; Apple: [I-0327])
- [x] **E2** ✅ [SP-230] (**G8**, [T-0249]) — Go to Scene, Chapter and Manuscript Start / End, in the menus with the Mac's keys (✅ re-ruled for Linux: Manuscript Start / End on Ctrl+Home / Ctrl+End)
- [x] **E3** ✅ [SP-230] (**G9**) — Navigator search, as the Mac's (T-0569): titles and text, matches in closed chapters shown

*F. The project window's toolbar — new 2026-11-21 (user, 2026-11-21: *"I'd like to scan the Epic for any tasks that involve creating a menu bar of buttons across the top like macOS and iOS have.  If missing, I'd like to make sure its added to the Epic as a new Sprint and AC."*):*
- [x] **F1** ✅ [SP-232] ([I-0244]'s toolbar gap, [T-0694]) — a toolbar across the top of every project window, as the Mac's
      (`ProjectToolbar.swift`): Inspector, Timeline, Project Settings, Worlds, Navigate ▾, New ▾, Merge ▾, View ▾, Import ▾, Export ▾,
      Close Project — each the menus' own action. Linux has no `QToolBar` today. → [SP-232]

*Standing (EP-035 AC6–AC9, EP-036 AC5–AC8, EP-037 AC7–AC10):* the Porting Outline corrected from what each part costs; the
platform-independent rules re-honoured, not re-decided (kind scope derived, edge labels read, patch never reconstruct,
absence is never deletion, disabled and explained); ctest run non-root with tests on; a live pass per surface-shipping Sprint
(the Linux VM counts, per [EP-057]).

**Order (provisional):** E and D first (small; E1 is the user's example), then F (its View item needs D3), then A → B → C, as the 2026-08-25 split ordered them.
✅ **Part A re-ordered 2026-11-21** (user: *"Stack first"*): [SP-233] the card stacks (B5, B6, A7) → the object verbs on a card (A3–A6)
→ A8 (`[Apple]` delete / promote) → A9 (Linux, after A8) → the rest of B → C.
Sizes from the review: A–C about an Epic's worth on their own (Apple's EP-030 / 031 / 034 took 8+ Sprints); D a Sprint; E a
Sprint; F a Sprint.

*Last Updated: 2026-11-23 (✅ Closed.)*
