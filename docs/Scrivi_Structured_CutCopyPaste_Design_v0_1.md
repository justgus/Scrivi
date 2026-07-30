# Scrivi — Structured Cut / Copy / Paste Design (v0.1)

**Epic:** EP-029 `[Cross]` — Cross-Boundary Structured Cut / Copy / Paste
**Sprint:** SP-085 (design) — Task **T-0350**
**Status:** ✅ **APPROVED (user, 2026-07-27).** Trades ruled — **T1=A · T2=A · T3=A · T4=A** (the recommended
set); Open Questions #1–#3 resolved (§10). This is now the binding source of truth for SP-086–SP-089. Per
CLAUDE.md, implementation follows this doc; any deviation is surfaced + reconciled first.
**Date:** 2026-07-27
**Author:** Claude (planning), for user review.

> **Trade rulings (user, 2026-07-27):**
> - **T1 = A** — JSON `scrivi.fragment.v1` ordered pieces (§3).
> - **T2 = A** — ScriviCore-owned internal clipboard; the system pasteboard carries only the flat `plainText`.
>   (⇒ **Open Question #1 settled: NO cross-window / cross-project structured paste in v1** — internal-only;
>   cross-project paste stays out of scope per §2.)
> - **T3 = A** — one reversible structural event each (`structuredCut` / `structuredPaste`); undo = inverse op (§5).
> - **T4 = A** — extend `scrivi.buffers.v1` in place with an optional `fragment` field (§6); no v2, no migration.

**Supersedes / relates to:**
- Extends the copy-buffer feature delivered in **EP-019 SP-056** (`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §9).
- **Reuses** EP-027 (filesystem-authoritative scene/chapter identity & ordering) and EP-028
  (`SceneMerger` / `ChapterMerger` primitives). Does **not** re-implement identity, ordering, or merge.
- Structural undo remains **out of EP-019 scope** (EP-019 records barriers only); EP-029 introduces the
  *first* structural edits driven by clipboard operations, so it owns their history shape (§5).

---

## 1. Problem statement

The macOS manuscript editor presents the whole manuscript as **one continuous `NSTextView` / `NSTextStorage`**
(`Scrivi/Views/ManuscriptTextView.swift`). Inside that single storage:

- **Scene boundaries** are a **1-character divider attachment** inserted between adjacent scenes
  (`makeDividerAttachment()`), tracked by `coordinator.sceneBoundaries: [NSRange]` (one range per loaded scene;
  dividers sit *outside* those ranges).
- **Chapter boundaries** additionally carry a **non-editable heading run** tagged with the custom attribute
  `NSAttributedString.Key.scriviHeading` (`"scrivi.heading"`), rendered bold and guarded against editing.

Because of this, a selection that **spans a scene or chapter boundary** cannot be handled by the plain-text
Cut/Copy/Paste path the writer expects:

1. **⌘C across a boundary** pulls the divider attachment character and/or the non-editable heading text into the
   copied string, polluting it (a stray object-replacement char `U+FFFC`, or a literal "Chapter 3" heading).
2. **⌘X across a boundary** is silently blocked — the heading-edit guard rejects the mutation, so the cut is a
   no-op with no feedback.
3. **The copy buffers** (`scrivi.buffers.v1`, SP-056) store **flat text only**, so even if a clean cross-boundary
   string were extracted, pasting it back would drop all scene/chapter structure — the paste would dump many
   scenes' worth of prose into whichever single scene the caret sits in.

The writer's mental model is that **the manuscript is one document**. Selecting from the middle of Scene 4
through the middle of Scene 7 (crossing into Chapter 2 on the way) and pressing ⌘X should behave exactly as it
would in a single flat document: the text is removed, what remains on either side of the cut **joins up**, and
the removed span — *with its internal structure* — is available to paste elsewhere, **rebuilding those scenes
and chapters** at the destination.

**Goal:** make Cut / Copy / Paste and the copy buffers treat the manuscript as one monolithic document, carrying
and reconstructing scene/chapter structure across boundaries, while leaving the fast single-scene path (verified
in SP-056) untouched.

---

## 2. Scope

### In scope (this Epic)

- A **structured fragment** model (`scrivi.fragment.v1`) that captures a manuscript range crossing 0+ scene and
  0+ chapter boundaries, including partial head/tail scenes.
- **Extract-fragment** (ScriviCore): manuscript range → structured fragment (SP-086).
- **Paste-splice** (ScriviCore): insert a fragment at a caret, splitting the target scene mid-scene and creating
  the carried scenes/chapters (SP-087).
- **Cut-with-merge** (ScriviCore): delete a spanned range and **merge** the spanned scenes/chapters atomically,
  composing the existing `SceneMerger` / `ChapterMerger` (SP-088).
- **Copy-buffer schema evolution** so buffers hold structured fragments (T4).
- **Apple editor wiring** (SP-089): route ⌘C/⌘X/⌘V + ⌘/⌃/⌥1–9 through the fragment API **only when a selection
  or paste actually crosses a boundary**; keep the single-scene fast path; integrate with EP-019 history.

### Out of scope

- **Linux / Windows wiring.** The core capability is `[Cross]` and reusable, but this Epic wires **Apple first**
  (mirroring the EP-019 "Apple as reference" decision). Linux/Windows are follow-on Epics once those editors
  reach parity. `platforms/linux/` is untouched by EP-029.
- **iOS/iPadOS** capture (editor is a stub).
- **General structural undo** beyond clipboard-driven operations — EP-029 owns undo/redo of *its own* cut-merge
  and paste-splice (AC6); it does not retro-add undo to scene/chapter create/delete/reorder outside that path.
- **Cross-*project* paste.** A fragment carries no project identity; pasting a fragment copied from Project A
  into Project B is a v1 non-goal (the fragment is an in-session / same-project transport). Revisit if requested.

---

## 3. The fragment model (`scrivi.fragment.v1`)

A **fragment** is an ordered list of **pieces**, each piece corresponding to one scene's worth of contiguous
selected text, annotated with the boundaries that precede it. It is the transport for both the internal clipboard
and (per T4) the copy buffers.

```jsonc
{
  "schema": "scrivi.fragment.v1",
  "pieces": [
    {
      // Boundary that opens this piece, relative to the piece that precedes it in `pieces`.
      // "none"    — same scene continues (only valid for the FIRST piece: it means the
      //             selection started mid-scene; never appears after piece 0).
      // "scene"   — this piece begins a new scene (a divider was crossed).
      // "chapter" — this piece begins a new scene AND a new chapter (a heading was crossed).
      "opensWith": "none" | "scene" | "chapter",

      // Present only when opensWith == "chapter": the heading text as displayed
      // ("Chapter 3" or a custom title). Informational for reconstruction / external
      // flattening; the real chapter identity is minted fresh on paste (§4.2).
      "chapterTitle": "Chapter 3",

      // The selected text of this piece (this scene's contribution to the selection),
      // with divider/heading characters already stripped. May be empty (e.g. a boundary
      // was crossed but zero characters of the following scene were selected).
      "text": "…prose…",

      // True if this piece is a PARTIAL scene at the fragment's leading or trailing edge:
      //   head — the first piece started mid-scene (its scene's prefix stays behind on cut).
      //   tail — the last piece ended mid-scene (its scene's suffix stays behind on cut).
      // Interior pieces are always whole scenes (partial: null). Drives cut-merge (§4.3)
      // and paste-splice (§4.2): partial pieces splice into the neighbour rather than
      // creating a standalone scene.
      "partial": "head" | "tail" | null
    }
    // …one piece per scene the selection touches, in reading order…
  ],

  // A plain-text flattening of the whole fragment (pieces joined by a blank-line seam),
  // carried so the system pasteboard / external apps get clean text (T2). Derived, but
  // stored so consumers need not re-derive.
  "plainText": "…scene 1 text…\n\n…scene 2 text…\n\n…"
}
```

### 3.1 Invariants

- `pieces` is non-empty. `pieces[0].opensWith` is `"none"` for a selection that began mid-scene, or
  `"scene"`/`"chapter"` if the selection began exactly at a boundary.
- `opensWith == "none"` appears **only** at `pieces[0]`.
- A single-scene selection is a **one-piece fragment** with `opensWith: "none"`, `partial: "head"` **and**
  `"tail"` semantics collapsed — but by AC5 the editor never *builds* a fragment for a single-scene selection;
  it uses the flat path. The one-piece fragment is defined here only so the model is total.
- `chapterTitle` is present **iff** `opensWith == "chapter"`.
- No scene/chapter **IDs** are stored. Identity is **minted on paste** via EP-027 primitives (§4.2). A fragment
  is structurally descriptive, not identity-bearing — this is what keeps it same-project-agnostic and free of
  stale-ID hazards.

### 3.2 Why pieces, not a marked-up string (previews T1)

Two representations were considered (see Trade **T1**). The JSON-pieces form (option A) is recommended because it
makes the three core operations trivial index walks (`pieces[i]`), keeps the divider/heading stripping at
*extract* time (one place), and lets `partial` be an explicit flag rather than a parse of in-band sentinels.
The alternative in-band-marker string (option B) re-introduces exactly the "sentinel chars leak into text"
hazard this Epic exists to remove.

---

## 4. The three core operations (ScriviCore)

All three live in ScriviCore behind the C ABI, so every platform reuses them. Swift never manipulates fragment
structure directly — it hands ScriviCore a range (extract), a fragment + caret (paste), or a range (cut) and
receives back a fragment and/or a fresh scene list.

### 4.1 Extract-fragment (SP-086)

**Input:** the projectID + an ordered list of `(sceneID, startByteOffset, endByteOffset)` spans describing the
selection, scene by scene (the editor computes these from `coordinator.sceneBoundaries` — it already knows the
storage-range → sceneID map). ScriviCore does **not** need to know about `NSTextStorage`; it works from
scene-local byte offsets, consistent with the EP-019 history offset convention (§4.b of the undo/redo design).

**Output:** a `scrivi.fragment.v1` (§3), with divider/heading characters excluded by construction (the editor
never includes them in a span; ScriviCore validates the spans fall within scene bodies).

**Notes:**
- Extract is **read-only** — it never mutates the project. ⌘C is non-destructive.
- Chapter boundaries are detected from the scenes' chapter membership (EP-027 model), not from the heading text.
- `plainText` is computed here.

### 4.2 Paste-splice (SP-087)

**Input:** projectID + fragment + a caret `(sceneID, byteOffset)`.

**Behaviour** — reconstruct every carried boundary at the caret, as if inserting into a flat document:

1. **Split the target scene at the caret** (unless the caret is exactly at a scene start/end). The target scene
   becomes *head* (prefix) + *tail* (suffix). This composes the existing mid-scene split path (the one
   `Ctrl+Return` / EP-027 uses) rather than a new primitive.
2. **First piece** (`opensWith: "none"`, `partial: head`) appends to the *head*.
3. **Each subsequent `opensWith: "scene"` piece** → `createScene` after the running insertion point (EP-027
   `createScene` with order-key placement). `opensWith: "chapter"` piece → `createChapter(afterChapterID)` +
   its first scene (EP-027 `createChapter` in-place primitive, SP-071).
4. **Last piece** (`partial: tail`) → its text is **not** a standalone scene; it joins the *running insertion
   point* (the last scene created, or the target scene if the fragment had no boundaries). The original target
   scene's **tail-suffix follows the whole pasted run** — it is appended to that same running scene, *after* the
   tail piece's text. (Flat-document model, user-ruled 2026-07-27: the text after the caret must read continuously
   *after* everything pasted, e.g. target `"AAA|BBB"` + fragment `[none:"one", scene:"two", tail:"three"]` →
   `Scene1 "AAAone"`, `Scene2 "two"`, `Scene3 "threeBBB"`. NOT rejoined into the original scene, which would
   fracture reading order.)
5. Fresh sceneIDs/chapterIDs are minted by the create primitives; order-keys are assigned by the EP-027 model.
6. **In-scene joins are direct concatenation, not a blank-line seam** (user-ruled 2026-07-27). The two joins that
   happen *within* a scene — head + first-piece, and last-piece + tail-suffix — continue the paragraph at the
   caret (a flat-document insert), so no `\n\n` is inserted. The blank-line seam that `SceneMerger` uses is *not*
   used anywhere in paste: every cross-boundary piece becomes its own scene, so the scene split itself is the
   boundary. (Contrast SP-088 cut-merge, which *does* use the seam when collapsing former scenes back into one.)

**Output:** the updated scene list (so the editor rebuilds its segments) + the set of created sceneIDs/chapterIDs
(so history/undo can target them, §5).

**Edge cases:**
- Caret at a scene **start** → no split; head is empty; first piece prepends.
- Caret at a scene **end** → no split; tail is empty; last piece appends.
- A one-piece fragment pasted → a plain text insert (fast path; the editor won't even call paste-splice for it).
- **Caret in a non-editable chapter heading** → **paste is refused.** The editor guards the caret *before*
  calling paste-splice: if the insertion point sits inside a `scriviHeading` run, it **flashes the screen**
  (`NSBeep()` + a brief visual flash), does **not** paste, and leaves the caret where it is so the writer can
  reposition into a scene. Paste-splice is never invoked with a heading caret (§10 Q2 — user ruling 2026-07-27:
  do **not** silently retarget the paste; stop and signal). This is an editor-side (SP-089) precondition.

### 4.3 Cut-with-merge (SP-088)

> **⚠️ SUPERSEDING DECISION (2026-07-29, user ruling — T-0357).** The original delete-and-fold behaviour below
> **left the tail chapter's post-span scenes in place**, which produced **duplicate chapter titles** in the
> manuscript (cut into "The Thing in the Black Casket", paste it back → two chapters both titled "The Thing in
> the Black Casket"). To eliminate that litter, cut-with-merge now **captures chapter identity into the fragment
> and promotes the tail chapter's survivors into the head chapter.** See **§4.4 Title-capture & chapter
> promotion** immediately after this section — it governs; the delete-and-fold text below is retained as the
> pre-2026-07-29 record and still describes the same-chapter (no boundary) and text-fold mechanics, which are
> unchanged.

**Behaviour** (implementation ruled 2026-07-27 — **delete-and-fold**, see below):

1. **Extract** the fragment first (so the removed content is available to paste / land in a buffer).
2. **Fold the surviving text into the head scene:** the head scene's body becomes
   `headPrefix + tailSuffix` — the bytes before the span start abutted **directly** to the bytes after the span
   end. **No blank-line seam** (user-ruled 2026-07-27): a cut just deletes the selected characters and closes the
   gap, exactly like deleting a selection in any editor; the surviving edges abut as-is (whatever whitespace the
   writer's selection left). (This corrects an earlier draft that reused `SceneMerger`'s seam — a *merge* of two
   distinct scenes seams; a *cut* of a selection does not.)
3. **Delete every other scene in the span** — all interior scenes *and* the tail scene — by sceneID
   (`SceneDeleter`). Scenes *after* the span in a partially-covered chapter are left untouched.
4. **Remove any chapter left empty** by step 3 (all its scenes were in the span) via `ChapterDeleter`.

The result is **one scene** containing `headPrefix + tailSuffix`, in the head scene's chapter, with all interior
scenes and any emptied chapters removed.

**Output:** the extracted fragment + the set of removed sceneIDs/chapterIDs (for undo) + the surviving head
sceneID.

**Why delete-and-fold rather than "compose SceneMerger/ChapterMerger" (ruling 2026-07-27):** the merge
primitives don't fit a general span. `SceneMerger` is **same-chapter only**; `ChapterMerger` merges a **whole
chapter** (it would wrongly pull in scenes *after* the span when the span ends mid-chapter); and there is no
single-scene-cross-chapter merge primitive. Delete-and-fold is uniform across same-chapter and cross-chapter
spans, leaves post-span scenes untouched, and is still "merge across N scenes" — expressed as *fold the two
survivors + delete the rest*. It composes the existing `SceneWriter` / `SceneDeleter` / `ChapterDeleter`
primitives (no new structural code), performed as a single ScriviCore call so the whole collapse is one
operation from the app's view (atomicity: each primitive is individually atomic; the call fails fast on the first
error, and because deletions run after the head fold, a mid-sequence failure leaves the head scene already
carrying all surviving text — no text is lost).

### 4.4 Title-capture & chapter promotion (SP-089 · T-0357, supersedes §4.3 for chapter-crossing cuts)

**Motivation (user, 2026-07-29):** a cut that consumes a chapter's heading region should *take that chapter's
name with it* into the cut buffer, so the writer never ends up with two chapters wearing the same title. When
the name is later pasted, it comes back on the pasted chapter; the chapter it was cut from — if it still holds
scenes — carries **no stored title** and therefore displays by **positional default** ("Chapter 2", "Chapter 3",
… — the manuscript already renders an untitled chapter as `Chapter <ordinal>` and renumbers automatically).

**What the fragment carries.** Every piece records its **scene title** (`sceneTitle`); a piece that opens a
chapter also records that **chapter's title** (`chapterTitle`, already in the model). These travel through
cut → buffer → paste so paste can restore the original names (previously both were discarded → paste minted
"Chapter N" / untitled scenes).

**Cut, when the span crosses ≥1 chapter boundary** (a piece has `opensWith == Chapter`):
1. Extract the fragment (now capturing scene + chapter titles) and fold the two edge scenes as in §4.3.
2. **Promote the surviving post-span scenes** of the last partially-cut chapter **into the head chapter** —
   relocate their files after the head chapter's last scene (reusing the `ChapterMerger` relocation primitive
   built for I-0083, which mints order keys and moves the `.md`/`.meta.json` atomically).
3. **Delete every now-emptied chapter** in the span — both fully-consumed middle chapters and the drained tail
   chapter. Result: **one continuous chapter** (the head), no duplicate-named remnant.
4. A same-chapter cut (no `opensWith == Chapter` piece) is **unchanged** — plain delete-and-fold per §4.3.

**Paste** applies the captured identity: a piece opening a chapter creates that chapter **with its captured
`chapterTitle`**; every created scene is named from its captured `sceneTitle`. Chapters below the insertion point
renumber positionally on their own (no stored-title rewrite needed). A pasted chapter therefore reappears as
"The Thing in the Black Casket" while the chapter that now sits where it used to be shows the positional default.

**Undo (T3):** still a **barrier** for now (undo stops with a notice), consistent with §5 and the T-0356
carve-out. Full reversible undo — which must also restore the promoted scenes' original chapter membership and
the survivor chapter's original title — lands with **T-0356**. (User ruling 2026-07-29: keep the barrier for
this task.)

---

## 5. History integration (EP-019)

Cut-merge and paste-splice are the **first structural edits EP-029 introduces**, so this Epic owns their history
shape. EP-019 records structural operations as **barriers** (undo stops at them with a notice). Trade **T3**
decides whether EP-029 keeps that barrier model or records a single reversible structural event.

**Recommended (T3 = A, single reversible event):**
- A cross-boundary **cut** records **one** history event of a new kind `structuredCut` carrying enough to restore
  both the text **and** the pre-merge structure (the fragment + the removed sceneIDs/chapterIDs + their order-keys
  + the caret). Undo re-creates the scenes/chapters and re-inserts the text (i.e. undo of a cut == a paste-splice
  of the extracted fragment at the cut point). Redo re-runs the cut.
- A structured **paste** records one `structuredPaste` event carrying the created sceneIDs/chapterIDs + the caret;
  undo removes exactly those created scenes/chapters and re-joins the split target (i.e. undo of a paste ==
  cut-merge of the pasted span). Redo re-runs the paste.

This makes AC6 ("undo of a cut restores text AND structure; undo of a paste removes created scenes") a direct
consequence: **undo-cut and undo-paste are each other's inverse operation**, which is why composing them on the
same extract/paste-splice/cut-merge primitives is clean. The alternative (T3 = B, barrier + sub-events) is
simpler to build but leaves the writer unable to undo a mis-cut in one step — a poor fit for a clipboard action.

**Buffer history (unchanged from SP-056, Trade T3 of EP-019):** copy-into-buffer records **no** event; a
cut-into-buffer records the `structuredCut` event tagged with its `bufferID`; paste-from-buffer records the
`structuredPaste` event.

---

## 6. Copy-buffer schema evolution (T4)

Buffers today (`scrivi.buffers.v1`, `BufferStore`) store `{ bufferID, label, text, updatedAt }`. To hold a
structured fragment they must carry the fragment too. Trade **T4** decides in-place vs a version bump.

**Recommended (T4 = A, extend `scrivi.buffers.v1` in place — additive):**
```jsonc
{
  "bufferID": "3",
  "label": null,
  "text": "…flat plainText…",      // still present; the flattening of `fragment`
  "fragment": { "schema": "scrivi.fragment.v1", … },  // NEW, optional; absent for plaintext-only slots
  "updatedAt": "…"
}
```
- A slot with **no** `fragment` is a plain-text slot exactly as SP-056 wrote it — **old buffers files load
  unchanged**, and single-scene copies keep writing text-only slots (AC5).
- `text` remains the authoritative flat form (system-pasteboard / external paste), so a v1-only reader still
  works. Adding an optional field is backward-compatible under the existing "corrupt/unknown ⇒ treated as empty"
  tolerance in `BufferStore::readAll`, but here it's not even lossy — old readers ignore `fragment`, new readers
  use it.
- No `scrivi.buffers.v2` migration is needed. If a future change is non-additive we bump then; today it isn't.

`BufferStore::load` gains an overload / the C ABI gains a parameter to accept an optional fragment JSON alongside
the text (detailed at SP-089 planning; the store change is small and additive).

---

## 7. Trade studies (to be ruled — **user decision required before implementation**)

Each trade lists options, a recommendation, and the rationale. **All four ruled by the user 2026-07-27:
T1=A · T2=A · T3=A · T4=A** (the recommended set). Rulings are binding on SP-086–SP-089.

### T1 — Fragment format
- **(A) JSON `scrivi.fragment.v1` — ordered pieces (§3).** ✅ **Recommended. ✅ RULED (user, 2026-07-27).** Structure is explicit data, not
  parsed sentinels; the three operations are index walks; stripping happens once at extract; round-trips cleanly
  through the buffer schema (T4) and the pasteboard flattening (T2).
- (B) A lightweight in-band marker string (sentinels between scenes/chapters). Compact, but re-introduces the
  "control chars leak into prose" failure mode this Epic exists to remove, and makes `partial` a parse.
- (C) Reuse the on-disk scene/chapter layout as the transport (serialize a mini-project). Faithful but heavy;
  couples the clipboard to the package format and drags identity/order-keys into a transport that shouldn't carry
  them (§3.1). Overkill for a clipboard.

### T2 — Internal clipboard vs system pasteboard
- **(A) ScriviCore-owned internal clipboard holds the fragment; the system pasteboard carries only the flat
  `plainText`.** ✅ **Recommended. ✅ RULED (user, 2026-07-27)** — internal-only; **no cross-window/cross-project
  structured paste in v1** (Open Question #1 settled). External apps still get clean text; internal paste gets full structure; the
  system pasteboard is never polluted with a custom blob. Matches the SP-056 "system pasteboard is buffer 0,
  never clobbered" principle.
- (B) A custom `NSPasteboard` UTI carries the fragment on the system pasteboard. Enables cross-*app*/cross-window
  structured paste, but coupling the transport to `NSPasteboard` is Apple-specific (this is a `[Cross]` core) and
  risks a malformed blob surviving on the OS pasteboard across app quits.
- (C) Both (A internal + B UTI). Most capable, most surface area. Defer the UTI to a later Epic if cross-window
  structured paste is actually wanted.

### T3 — Cut-merge / paste undo granularity
- **(A) One reversible structural event each (`structuredCut` / `structuredPaste`); undo is the inverse
  operation (§5).** ✅ **Recommended. ✅ RULED (user, 2026-07-27).** A clipboard action should undo in one step, text + structure together;
  the inverse-operation framing falls straight out of the shared primitives.
- (B) A history **barrier** + sub-events (the current EP-019 structural model). Simpler, consistent with existing
  barriers, but the writer can't one-step-undo a cross-boundary cut — surprising for ⌘Z after ⌘X.

### T4 — Copy-buffer schema evolution
- **(A) Extend `scrivi.buffers.v1` in place — optional `fragment` alongside `text` (§6).** ✅ **Recommended.
  ✅ RULED (user, 2026-07-27).** Additive, backward-compatible, no migration; plaintext slots and old files keep working (AC5).
- (B) Bump to `scrivi.buffers.v2` (fragment-aware) with a v1→v2 migration on open. Cleaner conceptually but the
  change is purely additive, so a version bump + migration is churn without benefit; reserve v2 for a
  non-additive change.

---

## 8. Acceptance criteria (mirrors EP-029; finalised here)

- **AC1** — ⌘C across boundaries places a `scrivi.fragment.v1` on the internal clipboard; the system pasteboard
  gets the clean `plainText` flattening (blank-line seams) for external apps. (T1=A, T2=A.)
- **AC2** — ⌘V of a structured fragment reconstructs every carried scene/chapter boundary at the caret, splitting
  the target scene mid-scene; identity/order-keys via EP-027; merge/split via EP-028 primitives. (§4.2.)
- **AC3** — ⌘X across boundaries copies the fragment, deletes the span, and **merges** the spanned scenes/chapters
  into one, atomically; recorded as one undoable structural operation. (§4.3, §5, T3=A.)
- **AC4** — Copy buffers hold structured fragments: ⌘1–9 copy / ⌥1–9 cut across boundaries store structure;
  ⌃1–9 paste reconstructs it. `buffers.json` evolves per T4. (§6.)
- **AC5** — Within a single scene, Cut/Copy/Paste and the buffers keep the existing fast plain-text path — no
  regression to the SP-056 behaviour verified 2026-07-27. The fragment path engages **only** when a selection or
  paste actually crosses a boundary.
- **AC6** — Undo/Redo of a cross-boundary cut restores text AND pre-merge structure; undo of a structured paste
  removes the created scenes/chapters. No text or structure corruption. (§5.)
- **AC7** — No regression: auto-save, scene navigation, external-change scan, Git snapshots, and the EP-019
  history engine unaffected; ScriviCore `ctest` + interop suites green.

---

## 9. Sprint / milestone breakdown

| Sprint | Deliverable | Codebase |
| ------ | ----------- | -------- |
| **SP-085** | **This design doc + T1–T4 ruled + fragment schema spec** (T-0350) | `[Cross]` (docs) |
| SP-086 | ScriviCore `scrivi.fragment.v1` model + **extract-fragment** + C ABI (`scrivi_fragment_extract`) + unit/integration tests | `[ScriviCore]` |
| SP-087 | ScriviCore **paste-splice** (`scrivi_fragment_paste`) — split target + create carried scenes/chapters via EP-027; tests | `[ScriviCore]` |
| SP-088 | ScriviCore **cut-with-merge** (`scrivi_fragment_cut`) — atomic delete+merge composing EP-028; `structuredCut`/`structuredPaste` history shape; tests | `[ScriviCore]` |
| SP-089 | `[Apple]` editor wiring — route ⌘C/⌘X/⌘V + ⌘/⌃/⌥1–9 through the fragment API on boundary-crossing; single-scene fast path preserved; buffer schema (T4); history integration; **verify AC1–AC7** | `[Apple]` |

**Boundary-change note:** `scrivi.h` gains a `scrivi_fragment_*` family (extract/paste/cut) across SP-086–088 —
an additive, documented C ABI growth (the same pattern as `scrivi_buffers_*` in SP-056), **not** a boundary
strategy change. Each new endpoint returns a JSON-over-`std::string` envelope. No new Apple-target `.cpp/.hpp`
(ScriviCore files go in CMake, not pbxproj); SP-089 adds Swift files (pbxproj updated in the same step).

---

## 10. Open questions (fold into the ruling)

1. **T2 cross-window paste.** ✅ **RESOLVED (user, 2026-07-27): NO** cross-window/cross-project structured paste
   in v1 (T2=A, internal-only). Cross-project paste is explicitly out of scope (§2).
2. **Caret-in-heading paste.** ✅ **RESOLVED (user, 2026-07-27): REFUSE the paste.** If the caret sits in a
   non-editable chapter heading when ⌘V fires, the editor **flashes the screen** (`NSBeep()` + visual flash),
   does **not** paste, and waits for the writer to reposition into a scene. Do **not** silently retarget to the
   chapter's first scene. Editor-side precondition, SP-089 (§4.2 edge cases).
3. **Selection anchored on a divider.** ✅ **RESOLVED (user, 2026-07-27): confirmed.** If a selection
   starts/ends exactly on a divider attachment, the editor normalises the span to the adjacent scene body before
   calling extract. Editor-side normalisation, SP-089.

---

*End of v0.1 draft. Awaiting T1–T4 rulings and approval.*
