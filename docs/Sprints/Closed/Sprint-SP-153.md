---
sprint: SP-153
epic: EP-045
status: Closed
closed: 2026-10-03
activated: 2026-10-03
platform: Apple
created: 2026-10-03
---

# SP-153 — `[Apple]` [EP-045] S1: Typed attachments, save fidelity, divider close-out

**Status:** ✅ **CLOSED 2026-10-03 (user-approved):** *"the live look passes, close SP-153"*. Created and activated 2026-10-03.
**Epic:** [EP-045] `[Apple]` The Manuscript Renderer — Foundations → [`../Epics/Epic-EP-045.md`](../Epics/Epic-EP-045.md)
**Design:** [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md) §2 (AC1), §3 (AC2)
**Size:** ✅ **SMALL** — one data-loss fix, its integration guard, one live look.

---

## Why these three, first

✅ **AC1 is the Epic's only data-loss item** and the design says *"do this FIRST"* (§2). ⚠️ `sceneBoundaries`
is what the save path slices scene bytes with, and today **every** attachment counts as a divider — so the
first non-divider attachment any feature adds would write the wrong bytes to the wrong scene files.
✅ **AC9 (save fidelity) is the guard that proves AC1 did not move a single byte**, so it ships with it.
✅ **AC2 needs only a Light-mode live look** — [T-0554] fixed the divider and the user verified it in
Dark on 2026-09-29.

⛔ **NOT in this Sprint:** AC3 (the SOURCE↔PRESENTED mapping) onward. ⛔ No escaping, no Enter/Backspace
changes, no rendering.

---

## ⚠️ Found at planning, read in the code (2026-10-03)

- ⛔ **AC1 is wider than the design says.** §2.2 names *"two call sites"*; ✅ **SIX code sites read
  `.attachment`** in `Scrivi/Views/ManuscriptTextView.swift` — `:1700` (`recomputeBoundaries`), `:2060`,
  `:2153`, `:2487` (`shouldChangeText` divider guard), `:2718`, `:2722` (separator test). ✅ **Every one must
  be typed**, or the untyped ones keep treating any attachment as a divider.
- ✅ **DESIGN RULED BY THE USER 2026-10-03:** *"dividers are not represented on ScriviCore. Therefore
  DividerTextAttachment is an object that renders in the ManuscriptView only. We can make the class manage
  different rendering states without changing the type. The attribute key can be managed via an enum and can
  represent the rendering state of the class."* ✅ **So: ONE class, rendering states owned by it; a
  `.scriviDivider` attribute key whose VALUE is the state enum; every reader tests the KEY.** ⚠️ The
  class-test alternative below is superseded.
- ~~✅ **The divider already has its own class** — `DividerTextAttachment` (`:2749`, from [T-0554]). ⚠️ The
  design proposed a new `.scriviDivider` attribute key; a class test (`value is DividerTextAttachment`) may
  serve instead. ⚠️ **Decide by MEASUREMENT, not preference:** check whether the subclass survives every
  path that rebuilds storage — undo (design trap #3: the undo path strips storage attributes), copy/paste
  (a pasteboard round trip can turn a subclass into a plain `NSTextAttachment`), and `rebuildStorage`.
  ⛔ If the class identity can be lost on any path, the attribute key is required.~~

---

## Acceptance Criteria

- [x] **AC1** — ⛔ **Only a divider attachment is a scene boundary, at ALL SIX readers.** ✅ Unit test: insert a
  non-divider attachment into a multi-scene manuscript → `sceneBoundaries` is unchanged and the six readers
  ignore it. ✅ The chosen mark (class or attribute) is shown to survive undo, paste and `rebuildStorage`.
  ✅ **Met 2026-10-03 — [T-0577]** (red against the old rule). ✅ **User-verified 2026-10-03 by live look.**
- [x] **AC9** — ✅ **Save fidelity:** a scene's bytes round-trip unchanged through edit → save → reload on a real
  temp project, proven through `scrivi_*` (not the facade — `feedback_boundary_tests_not_facade`).
  ✅ **Met 2026-10-03 — [T-0577].**
- [x] **AC2** — ✅ **Divider visible in LIGHT mode, by a live look** (Dark already user-verified via [T-0554]).
  → closes EP-045 AC2. ✅ **MET 2026-10-03 — user:** *"I did a live look at the dividers in light mode and they render correctly (i.e they are visible)."*
- [x] **AC-build** — macOS + iOS + visionOS build; interop tests green via `scripts/run-interop-tests.sh`;
  `ctest` green on macOS and Linux if the core changes. ✅ 2026-10-03: all three BUILD SUCCEEDED; 139/139; core unchanged.

---

## Progress log

### 🔵 2026-10-03 — Sprint CREATED in Planning

✅ Created at [EP-045]'s activation (user: *"Let's activate that epic next (EP-045?)."*). ⚠️ Not yet active.

### 🟡 2026-10-03 — Sprint ACTIVATED; AC2 MET; AC1 design ruled

✅ User: *"Ok."* ✅ **AC2 met** by the user's Light-mode live look. ✅ **AC1 design ruled** (above): one
`DividerTextAttachment` class with rendering states, marked by a `.scriviDivider` enum-valued attribute.

### 🟢 2026-10-03 — AC1 + AC9 implemented ([T-0577]); Sprint COMPLETE pending verification

✅ One `DividerTextAttachment`, `.scriviDivider` enum-valued key, `enum SceneDivider` as the single definition;
⛔ all SIX readers converted. ✅ Stray-attachment test RED against the old rule. ✅ AC9 byte-identical round trip
through `scrivi_*`. ✅ 139/139; three Apple builds. ⚠️ **Awaiting the user's live look ([T-0577]).**

### ✅ 2026-10-03 — [T-0577] VERIFIED; Sprint CLOSED (user-approved)

✅ *"the live look passes, close SP-153"*. ✅ [T-0577] archived → `../../Tasks/Verified/Task-verified-0577.md` in the
same step. ✅ **All four ACs met. [EP-045] AC1, AC2 and AC9 are MET.** Nothing carried forward.
