# Verified Tasks: T-0419 – T-0425 (SP-115)

✅ **All seven Verified by the user 2026-08-20.** Sprint **SP-115**, Epic **EP-034**.

| ID | Title | Issue | Priority | Verified |
| -- | ----- | ----- | -------- | -------- |
| **T-0419** | ⚠️ `lastKnownPackagePath` — carry the last-known candidate so AC24's refinement can fire | **I-0137** | **High** | 2026-08-20 |
| T-0420 | `world.json` `formatVersion` compared against a supported maximum | I-0136 | Medium | 2026-08-20 |
| T-0421 | Inline object editor — an exit that does not read as data loss | I-0139 | Medium | 2026-08-20 |
| T-0422 | Corrupt `world.json` test coverage | I-0135 | Low | 2026-08-20 |
| T-0423 | "Remove from scene" — disabled **and explained** | I-0138 | Low | 2026-08-20 |
| T-0424 | ⚠️ **FILE (do not fix)** the two kind-list findings → I-0140, I-0141 | — | Medium | 2026-08-20 |
| **T-0425** | ⚠️ Object editor shows its object's **own world, as a LABEL**; ⚠️ **rename of a world object was broken** | **I-0142** | **High** | 2026-08-20 |

**Suites at verification:** `ctest` **525/525** · macOS interop **103/103 in 10 suites** · app
**BUILD SUCCEEDED**.

---

## What each Task changed

**T-0419 (I-0137).** `WorldResolution` and `WorldSummary` gain **`lastKnownPackagePath`**, carried
**regardless of status**, through both C ABI envelopes and both Swift structs. ⚠️ **`packagePath` keeps its
meaning — *verified* — and was deliberately NOT widened**; the distinct name is the safeguard, because
`resolve` refuses to report a path it could not confirm (the I-0115 discipline). ⚠️ **Verified on the REAL
RIG**, drive ejected, showing *"on a disconnected volume"* where it previously said only *"unavailable"*.

> ⚠️ **Found and fixed a second gap while here:** `WorldStatusResult.worldStatus` returned the **raw** core
> status while its sibling `WorldEntry.worldStatus` refined — two accessors answering the same question and
> disagreeing.

**T-0420 (I-0136).** `parseWorld` refuses `formatVersion > kSupportedFormatVersion` with
**`unsupportedVersion`** + detail `unsupportedWorldFormatVersion` — *"too new" is not "damaged"*, and
callers must tell them apart. ⚠️ `resolve` reports such a world **`unavailable`, never `missing`**: the
package is plainly there, and a wrong *"missing"* invites destructive remedies against an intact world.

> ⚠️ **VERIFIED AT THE CORE ONLY, and that is knowingly incomplete.** Nothing in Scrivi surfaces the error,
> so a writer opening a too-new world sees *"unavailable"* with **no explanation**.
> **`project_capability_without_surface` inside the very sprint that fixed four other instances of it.**
> **Owed a writer-facing surface in a later sprint.**

**T-0421 (I-0139).** The exit is now named for what it **actually does** — `Done`/`Cancel` when nothing
changed, `Revert`/`Discard` (destructive-styled) only when work would be lost. Added
`ObjectDraft.hasUnsavedChanges`. ⚠️ **Deliberately not fixed** by making the panel modal (§4.6 forbids it),
by removing the inline editor (Q-b keeps it), or by a Cancel that bypasses the unfinished-work prompt —
that route is how **I-0119** filed an object into the wrong scene.

**T-0422 (I-0135).** Test proving a corrupt `world.json` degrades to **`unavailable`, not `missing`**, and
that the file is **neither regenerated nor deleted** — §6a.0's *absence is never deletion*.

**T-0423 (I-0138).** The disabled remove button now explains **why** via `pendingHelp`. ⚠️ The wording
*"Remove from scene"* is unchanged on healthy rows — **AC22: the edge goes, the object stays**, and
"Delete" would scare a writer off a non-destructive action.

**T-0424.** ⚠️ **Filed I-0140 and I-0141; fixed neither**, as scoped. Both are cured by design-doc **D5**'s
kind-scope endpoint in SP-116, and fixing them inside a five-Issue sprint would have blurred its boundary.

**T-0425 (I-0142).** ⚠️ **The one defect no suite found — the user found it during verification.**
`worldID` was gated on `pending` across **three** layers (`EndpointResolver:44-49`,
`RelationshipStore:490-494`, `scrivi_c_api.cpp:895`), so a reachable world object crossed the boundary with
no world attributed. ⚠️ **The visible symptom was the lesser half: `rename()` passes that `worldID` to
`openObject`, so renaming ANY world object was failing silently.** ✅ **User ruling: an object cannot be
moved between worlds** — the control is a **label**, and the questions a move would raise (migrate related
objects? cross-world edges? delete or remap them?) were deliberately left unopened.

---

## Lessons this sprint paid for

1. ⚠️ **A passing suite is not evidence a feature works in the product.** I-0137's refinement was correct,
   unit-tested and correctly wired — and could never fire, because a fixture supplies the one input the
   real product never does. **Only the ejected drive proved it.**
2. ⚠️ **A user's five-minute pass caught what 628 automated tests did not** (I-0142) — the same argument
   that made T-0418's live pass mandatory in EP-031.
3. ⚠️ **A control that implies a capability the code lacks is a defect even when nothing breaks.** The
   world picker offered a move `rename` could never have performed, since it patches `displayName` only.
4. ⚠️ **Fixing four instances of a defect class does not stop you shipping a fifth** — T-0420 landed a
   capability with no surface while the sprint around it fixed exactly that shape.

---

*Archived 2026-08-20 on user verification, in the same step the Issues were archived.*
