# Verified Task — T-0577

**Sprint:** [SP-153] · **Epic:** [EP-045] (AC1 + AC9) · **Platform:** `[Apple]`
**Implemented:** 2026-10-03 · ✅ **USER-VERIFIED 2026-10-03 by live look:** *"the live look passes, close SP-153"*
**Archived:** 2026-10-03, in the same step as the Sprint close.

---

## ✅ T-0577 — `[Apple]` Typed scene dividers + save-fidelity guard ([EP-045] AC1 + AC9) — Implemented 2026-10-03 · ✅ **VERIFIED 2026-10-03 (user)**

✅ **Sprint:** [SP-153] · **Epic:** [EP-045] · ✅ **Design ruled by the user 2026-10-03:** *"DividerTextAttachment is an
object that renders in the ManuscriptView only. We can make the class manage different rendering states without
changing the type. The attribute key can be managed via an enum and can represent the rendering state of the class."*

✅ **THE CHANGE (`Scrivi/Views/ManuscriptTextView.swift`):**
- `.scriviDivider` attribute key; its VALUE is `enum DividerRenderState { sceneBreak, chapterEnd }`.
- `DividerTextAttachment` keeps ONE type; `endsChapter: Bool` became `renderState: DividerRenderState` (the
  chapter-end tint reads it).
- `enum SceneDivider` is the ONE definition: `string(_:state:newlineAttributes:)` is the only place a divider is
  built (the key on the attachment character ALONE, never its "\n"), `isDivider(in:at:)` the only point test,
  `sceneBoundaries(in:)` the pure scan `recomputeBoundaries` now delegates to (behaviour unchanged).
- ⛔ **ALL SIX `.attachment` readers now test the key** — the scan, `storageOffsetToManuscriptPosition`,
  `caretOutsideSceneGap`, the `shouldChangeText` divider guard, and both halves of `isSeparatorPosition`.
  ⚠️ The design named two; six existed.

✅ **WHY THE MARK SURVIVES EVERY PATH (read in the code):** `rebuildStorage` is the only constructor and uses
`SceneDivider.string`; undo/redo's `applySceneChange` replaces ONLY a scene's own range (`:441-443`, dividers sit
outside it); paste is plain text (`isRichText = false`), so no attribute can arrive from outside. ⚠️ **So the
corruption path was LATENT, not live** — it goes live with the first feature that inserts an attachment.

✅ **TESTS (`ScriviInteropTests.swift`, suite "Typed scene dividers (EP-045 AC1)"):** divider splits + carries its
state; ⛔ **a stray non-divider attachment is NOT a boundary** — ✅ **RED against the old `.attachment` rule**
(temporarily restored: 1 failure), green with the key; an undo-style scene replace keeps the divider; headings
skipped + chapter-end state. ✅ **AC9:** three scenes with easily-mangled bytes (é, 👋, Markdown punctuation,
trailing spaces, blank lines, a tab, no trailing newline) saved through `scrivi_save_scene`, laid out as
`rebuildStorage` does (heading + dividers), sliced with `SceneDivider.sceneBoundaries`, saved again and reloaded —
✅ **byte-identical.** ✅ **139/139** via `scripts/run-interop-tests.sh`. ✅ macOS / iOS / visionOS BUILD SUCCEEDED.

⚠️ **Needs a live look:** open a multi-scene project — dividers still draw (chapter-end tinted), the caret still
skips the gap, ⌫/⌦ still refuse to delete a divider, and an edit saves to the right scene.
