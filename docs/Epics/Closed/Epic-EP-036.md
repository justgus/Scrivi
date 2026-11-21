# Epic EP-036 — ✅ CLOSED (superseded) 2026-11-20

**Closed:** ✅ **2026-11-20, SUPERSEDED by [EP-059]** (user, 2026-11-20: *"yes, file G1–G9 as an Epic (include everything here, so it is all tracked at once."*; asked how it relates to EP-035/036/037: *"One Epic for all"*).
Never activated. Every AC — verified and open — is carried into [EP-059] by reference; this file is the record of
what each AC meant and what it cost. ⚠️ The 2026-08-25 ruling that this work be THREE Epics (EP-035 §1) is superseded by
the same decision; EP-059 keeps the three as its parts A–C, in the same order.

---

## EP-036: `[Linux]` Object Detail Sheet & Media — ⚠️ **the second of three port Epics**

**Status:** 🔵 **Proposed** — created 2026-08-25 by the EP-035 three-way split.
⚠️ **WIDENED 2026-09-21 from the user's [SP-142] live pass: AC4a + AC4b** — ✅ **the writer's card
stack must be HONOURED (membership, order, and empty cards rendered), and there must be a PICKER to
compose it.** ⛔ **AC4b was in NO Epic at all.** ✅ **AC4a closes [I-0242].**
**Codebase:** `[Linux]` — Qt/QML over `ScriviBridge`. ⚠️ **No ScriviCore change expected.**
**Goal:** A writer can **open an object and see and edit what it actually is** on Linux — fields, notes,
tags, and imagery — and ⚠️ **never lose work when a world goes away.**
**Depends on:** ⚠️ **EP-035** — kind cards and the object list are the entry point this Epic opens FROM.
**Date Created:** 2026-08-25 · **Target Close:** — (⚠️ **3–4 sprints, estimated before implementation**)

⚠️ **Apple equivalent: ~1,350 lines** (`ObjectDetailSheet.swift` 816, `ObjectImageSection.swift` 368,
`ExistingAssetPicker.swift` 168) — ⚠️ **and that is the SwiftUI line count, not the Qt one.**

### Acceptance Criteria (draft — ⚠️ to be ruled at promotion)

- **AC1** — The sheet **views and edits** `displayName`, `subtitle`, `notes`, `tags`; ⚠️ **edits persist
  across app restart.** ⚠️ **PATCH the object, never reconstruct it** (T-0436/T-0437) — a surface built
  before its typed model drops fields.
- **AC2** — An image can be **imported, displayed, replaced, removed**; ⚠️ for a **world-scoped** object
  it is stored **in the world package** and is present when that world opens **from a different project.**
- **AC3** — ⚠️ **A pending object opens READ-ONLY, explained, never written.**
- **AC4** — ⚠️ **A world going unavailable WHILE A SHEET IS OPEN loses no data.**
  ⚠️ **BLOCKED ON EP-038** — this AC cannot be written honestly until the real-hardware rig reports what
  a Linux drive-loss actually does. ⚠️ **Do NOT specify it from the Apple implementation**; that code's
  own header records that the documented API lied. ⚠️ **On Apple this took
  SIX fixes** (I-0162, I-0165, I-0165b, I-0166, I-0167, I-0168): the sheet must survive the outage,
  explain it, warn that unsaved edits are memory-only, and ⚠️ **not discard them on reload, close, or
  navigation.** ⚠️ **Expect the same on Linux — this is the highest-risk AC in the port.**
- **AC4a** — ⚠️ **THE WRITER'S CARD STACK IS HONOURED — read from `inspector-layout.json`, not
  derived from what the scene happens to contain.** ⚠️ **ADDED 2026-09-21 from the user's live pass**
  (⚠️ see **[I-0242]**, which this AC closes). ✅ **USER RULING:** *"The writer chooses which Card
  elements to display and in what order they appear. Therefore, if the writer chooses to display an
  empty card in the stack, then it should be displayed with an appropriate 'No Objects to Display'
  message. Apple already does this. Linux must as well."*
  ⛔ **Three parts, all required:** ✅ **the stack's MEMBERSHIP** comes from `defaultStacks` / per-scene
  `scenes`; ✅ **its ORDER** likewise; ✅ **an EMPTY card still RENDERS**, with Apple's message
  (`ObjectCard.swift:450` — ⚠️ *"empty is a normal state, not an error"*).
  ⚠️ **THE DOCUMENT IS ALREADY THERE AND ALREADY LOSSLESS** — ✅ **[T-0537] proved Linux round-trips
  `defaultStacks` and `scenes` intact** — ⛔ **`InspectorLayoutStore` simply exposes only
  `selectedTab`.** ✅ **So this is a READER, not a new schema.**
- **AC4b** — ⚠️ **THE STACK PICKER: a writer can ADD and REMOVE cards from a stack on Linux.**
  ⚠️ **ADDED 2026-09-21 — it was MISSING FROM EVERY EPIC**, ✅ **found by the same live pass.**
  ⚠️ **Apple's card registry exists specifically to offer cards in a "+" menu**
  (`InspectorCard.swift:266` — *"Offered in the '+' menu; not in any default stack"*), ⛔ **and Linux
  has no such control at all**, ⚠️ **so a Linux writer cannot compose a stack even once AC4a lets one
  be read.** ✅ **`project_capability_without_surface`.**
  ⛔ **AC4b DEPENDS ON AC4a** — ⚠️ **a picker that writes into a document nothing reads changes
  nothing on screen.**
- **AC5** *(standing)* — ⚠️ **Porting Outline corrected from experience**, not merely followed.
- **AC6** *(standing)* — ⚠️ **Platform-independent rules re-honoured, not re-decided.**
- **AC7** *(standing)* — `ctest` **actually runs** in the container, ⚠️ **NON-ROOT, tests ON.**
- **AC8** *(standing)* — ⚠️ **A LIVE VNC click-through per surface-shipping sprint** (user ruling
  2026-08-25).

### ⚠️ Known traps

- ⚠️ **AC4 is where Apple bled.** Six separate data-loss routes into one surface, ⚠️ **all found by
  ejecting a drive or navigating away — none by a suite.**
- ⚠️ **`volumeIsRemovable`/`volumeIsEjectable` are UNRELIABLE** on the real rig; availability is decided
  by **volume-root mount presence** (`project_test_rig_tintagael_eskandar`).

---
