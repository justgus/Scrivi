# T-0453 – T-0458 — SP-120: Source creation + footnote text (R6)

**Sprint:** SP-120 · **Epic:** EP-034 · ✅ **All six Verified 2026-08-24** (user-approved, by use)
**Closes:** **AC8**, and ⚠️ **T-0365's write half — owed since SP-102.**

---

## What shipped

| Task | Delivered |
| ---- | --------- |
| **T-0453** | `attributes` read/write in `ObjectDetail` — the `[{"k","v"}]` wire form; ⚠️ **round-trip test written FIRST** |
| **T-0454** | Create a `source` **from the documented object** + attach the `cites` edge |
| **T-0455** | ⚠️ **Cite an EXISTING source** — the I-0164 mirror, named at planning |
| **T-0456** | ⚠️ **T-0365's second entry point** — the object surfaces its own sources → **the same `CitationPopover`** |
| **T-0457** | Citation fields (`author`/`year`/`publisher`/`page`/`url`/`accessed`) + footnote text; ⚠️ **the popup learned to SHOW them** |
| **T-0458** | ⚠️ **S11 written FIRST** — **operations as well as fields** |

**Files added:** `Scrivi/Views/Detail/ObjectSourcesSection.swift`, `Scrivi/Views/Detail/Citation.swift`.
**Modified:** `ObjectDetail.swift`, `SourcesCard.swift`, `ObjectDetailSheet.swift`,
`ScriviInteropTests.swift` (+5), `project.pbxproj` (both files × 3 app targets).

**Suites:** `ctest` **567/567** · interop **127/127** (was 122) · app **BUILD SUCCEEDED**.

---

## ⚠️ The sprint made NO core change, and that was the finding

Every backend piece of AC8 already shipped: the `source` kind (SP-098), `cites` unconstrained on both
ends (SP-096), `create_object`, `create_edge`, and a round-tripping `attributes` map — ⚠️ **the last of
these read by NOTHING since SP-095.**

⚠️ **`ctest` staying at 567/567 is the evidence**, not an absence of one: it confirms SP-120 built
**surface only**, which is what kept it `[Apple]`. This is `capability_without_surface` for the **third**
time inside EP-034.

---

## ⚠️ S11 was written first, and it earned its place three times

Ruling 3 widened S11 from **fields** to **operations**, because ⚠️ **fields-only is exactly how I-0164
slipped through SP-119.** It paid off immediately:

1. ⚠️ **It found T-0455 before a writer did.** "Cite an existing source" is an *operation*; no field table
   could surface it. Without it, citing one work across three characters silently builds three duplicate
   bibliography entries.
2. ⚠️ **It caught the trap INSIDE the sprint curing the trap.** `CitationPopover` read only
   `displayName`/`subtitle`/`notes`. Left alone, every citation field would have been written, stored,
   round-tripped — **and invisible in the very popup §3.1.1 calls "the record."**
3. ✅ **It shortened the work.** `notes` already **is** AC8's footnote text — long-form, already in the
   popup. ⚠️ **No new field was needed for that half**, which is worth stating rather than discovering.

---

## ⚠️ The `{"k","v"}` trap, and why the test came first

`attributes` serialises as an **array of pairs**, not an object (`ObjectJson.cpp:46-50`). Writing
`{"author": "…"}` parses back **empty** — every citation field silently dropped, with the file looking
perfectly reasonable to a human.

⚠️ **This is the identical shape trap T-0449 hit with `tags`.** The round-trip test was written before the
encoder, and a **negative control** proved the trap is real rather than hypothetical: the object-shaped
write decodes to nothing.

---

## ⚠️ Two defects, both from the live pass, neither from a suite

| Issue | What |
| ----- | ---- |
| **I-0169** | The `sources` card had **no route to the Detail Sheet** — ⚠️ **the writer's FIRST instinct.** `CardContext.openObjectDetail` already existed and this card never called it (`feedback_look_for_existing_pattern_first`). |
| **I-0170** | A twice-cited source **named only ONE citing object.** ⚠️ **The SP-120 wording decision was right; its implementation dropped information instead of re-centring it.** |

⚠️ **I-0170 is the more instructive failure: every field was present and populated, and the surface was
still quietly UNDER-REPORTING the writer's own graph.** A field-level review could not have caught it. It
took real data — a source cited **twice, across two different kinds** (a character *and* a location).

⚠️ **22 consecutive Issues across SP-118, SP-119 and SP-120 came from clicking, and none from a suite.**

---

## Verified by use

On `the-lone-golem.scrivi` — the sequel to `the-stairs-of-tintagael` — against the live **Eskandar**
world on USB: a new character (*Myton at 23*) with an image, documented by a source (*The Stairs of
Tintagael*) cited from **two** objects, edited on its own sheet, and read back from the Writing pane.

⚠️ **The `sources` card rendered CONTENT for the first time since it shipped in SP-102.** Until this
sprint it could only ever draw its empty state — indistinguishable from working correctly.

---

*Archived 2026-08-24 on user verification, in the same step the Tasks were marked Verified.*
