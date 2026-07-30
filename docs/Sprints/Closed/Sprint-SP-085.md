# SP-085: [Cross] EP-029 — Design doc + trade studies + fragment schema ✅ CLOSED

**Status:** ✅ **Closed 2026-07-27 (Human-approved).**
**Epic:** EP-029 `[Cross]` — Cross-Boundary Structured Cut/Copy/Paste (1st of 5 planned sprints; design/planning).
**Start / Close:** 2026-07-27 / 2026-07-27 (same-day design sprint).

## Goal (met)

Write and get approval for the EP-029 design doc — the `scrivi.fragment.v1` schema, extract / paste-splice /
cut-merge behaviour, copy-buffer schema evolution, history integration, the four trade studies, and the
SP-086–SP-089 milestone breakdown.

## Delivered

- **`docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md` — ✅ APPROVED (user, 2026-07-27).** Grounded in the real
  code, not speculation:
  - Boundary model read from `Scrivi/Views/ManuscriptTextView.swift` (one `NSTextStorage`; scene boundaries =
    1-char divider attachments; chapter boundaries = non-editable `scriviHeading` runs; `sceneBoundaries` range
    map).
  - Buffer schema read from `ScriviCore/src/history/BufferStore.{hpp,cpp}` (`scrivi.buffers.v1`).
  - Reused primitives confirmed in `ScriviCore/src/manuscript/` (`SceneMerger`, `ChapterMerger`, EP-028) +
    EP-027 create/split path.
- **Fragment model** `scrivi.fragment.v1` — ordered `pieces` (`opensWith` none/scene/chapter, `partial`
  head/tail, per-piece `text`, derived `plainText`), identity minted on paste (no IDs in the transport).
- **Three ScriviCore ops specified:** extract-fragment (read-only), paste-splice (split target + create carried
  scenes/chapters via EP-027), cut-with-merge (atomic delete + N−1 pairwise merges via EP-028).
- **History shape:** one reversible `structuredCut` / `structuredPaste` event each; undo = inverse operation.
- **SP-086–SP-089 milestone breakdown** locked.

## Trade rulings (user, 2026-07-27)

| Trade | Ruling | |
| ----- | ------ | - |
| T1 Fragment format | **A** | JSON `scrivi.fragment.v1` ordered pieces |
| T2 Clipboard location | **A** | ScriviCore internal clipboard + flat text on the system pasteboard (⇒ **no** cross-window/cross-project structured paste in v1) |
| T3 Cut/paste undo | **A** | one reversible structural event each; undo = inverse op |
| T4 Buffer schema | **A** | extend `scrivi.buffers.v1` in place (optional `fragment`); no v2, no migration |

## Open questions resolved (§10)

1. Cross-window/cross-project structured paste in v1 → **No** (T2=A, internal-only).
2. Caret-in-heading paste → **Refuse the paste**: flash the screen (`NSBeep()` + visual flash), do **not** paste,
   wait for the writer to reposition into a scene. (User overrode the drafted "silently retarget to first scene".)
3. Selection anchored on a divider → editor normalises the span to the adjacent scene body before extract. Confirmed.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0350 | Design doc: fragment schema, extract/paste-splice/cut-merge, buffer-schema evolution, trades T1–T4, milestone breakdown | ✅ **Verified (2026-07-27)** — doc written + trades ruled + approved |

## Retrospective

- **Went well:** design grounded directly in the editor/store/merge source before drafting, so the three ops map
  onto real primitives (EP-027 create/split, EP-028 merge) rather than new implementations. All four trades came
  back on the recommended set.
- **Caught early:** the "SP-086" in the kickoff message was a sprint-number typo (Epic plan has SP-085 = design);
  confirmed before activating anything, so no renumbering churn.
- **User override:** caret-in-heading paste is a **refuse + flash**, not a silent retarget — a safer, more
  predictable writer experience. Doc corrected before approval.
- **Next:** SP-086 — ScriviCore `scrivi.fragment.v1` model + extract-fragment + `scrivi_fragment_extract` C ABI +
  tests (activated on user go-ahead).

## Notes

- No code, no `scrivi.h`, no pbxproj changes (docs-only sprint). New file:
  `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`.
- Next available Sprint: **SP-086**. Next available Task: **T-0351**.
