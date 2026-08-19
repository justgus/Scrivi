# Verified Tasks: T-0298 – T-0301 — `[ScriviCore]` Scene & Chapter merge endpoints

**Status:** ✅ **Implemented - Verified**
**Sprint:** SP-074 (✅ Closed 2026-07-21, user-approved)
**Epic:** EP-028 `[Cross]` Scene & Chapter Merging (✅ Closed 2026-07-22, user-approved)
**Date Implemented:** 2026-07-20 – 2026-07-21
**Date Verified:** **2026-08-19 (user ruling)**
**Archived under:** audit remediation — [`../../Audits/Audit-Remediation-20260819.md`](../../Audits/Audit-Remediation-20260819.md)

> ⚠️ **These four Tasks were carried as 🟠 Implemented - Not Verified for four weeks after their Sprint
> and Epic had both closed.** Surfaced by the R-18 enumeration during the 2026-08-19 audit remediation,
> which found the conflict the records could not settle on their own:
>
> - **SP-074 closed 2026-07-21** (user-approved) — yet its task table and retrospective both still read
>   *"🟢 Implemented, Not Verified"*
> - **No `Verified/` file existed** for any of the four
> - ⚠️ `Sprint-SP-074.md` l.80 had set the condition: *"Do not close EP-028 or mark I-0083 fully Verified
>   until app adoption lands and is user-verified"* — **and EP-028 duly closed 2026-07-22**, one day later
>
> **Claude declined to resolve it** (Claude cannot mark a Task Verified) and referred it to the user, who
> ruled 2026-08-19: *"You may record these Tasks as Verified and make sure they are properly archived."*

---

## The four Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0298** | Reproduce chapter-merge data-loss (`MergeSceneTests.cpp`) + confirm same-chapter scene-merge coherence | ✅ Verified |
| **T-0299** | `scrivi_merge_scene` — `SceneMerger`, request/result, facade, C ABI, `scrivi.h`, CMake | ✅ Verified |
| **T-0300** | `scrivi_merge_chapter` — atomic cross-folder relocation + emptied-chapter removal (**fixes I-0083**) | ✅ Verified |
| **T-0301** | Merge integration coverage + register in `tests/CMakeLists.txt`; `ctest` green macOS + Linux | ✅ Verified |

---

## T-0298 — the red repro

`ScriviCore/tests/integration/MergeSceneTests.cpp`: the `[I-0083]` test replays the **old buggy
Swift-composed path** (in-memory reassign + `scrivi_delete_chapter`) on a 2-chapter project and asserts
the loss — reopen shows `size==1`, the chapter-2 folder gone, and scene 2's body deleted on disk.

⚠️ **Retained as a regression guard on the buggy composition, not inverted.** A companion test confirms
the same-chapter scene-merge path was already coherent.

## T-0299 — `scrivi_merge_scene(projectRootPath, sceneID)`

New `SceneMerger` (`src/manuscript/SceneMerger.{hpp,cpp}`): same-chapter join into the **previous** scene,
located by filesystem-authoritative sceneID scan (EP-027 §8.1). The survivor keeps its own order-key
files; the merged body is appended (blank-line separator, elided if either side is empty); merged files
are removed; the chapter's `scenes[]` cache is rebuilt. First-scene / empty / unknown sceneID →
`invalidArgument`.

## T-0300 — `scrivi_merge_chapter(projectRootPath, chapterID)` — ⚠️ THE FIX FOR I-0083

New `ChapterMerger` (`src/manuscript/ChapterMerger.{hpp,cpp}`): **relocates every scene file** of the
merged chapter into the predecessor's folder (`util::keyAfter` the predecessor's last scene;
`.meta.json` + `.md` renamed and the sidecar slug/contentPath rewritten — exactly `SceneReorderer`'s
cross-chapter move), rebuilds the survivor cache, **then** removes the emptied chapter via
`ChapterDeleter` + `rebuildIndexIfInconsistent`.

> ⚠️ **Files move BEFORE the folder is deleted — that ordering is the whole fix.**

First-chapter → `invalidArgument`.

## T-0301 — coverage + cross-platform green

11 tests in `MergeSceneTests.cpp`: same-chapter merge + empty-body elision; 4-scene/2-chapter whole-chapter
merge with order and bodies preserved; manuscript-start / first-in-chapter no-ops; empty/unknown ID errors;
reopen round-trips via `ManuscriptOrderResolver`. Registered in `tests/CMakeLists.txt`.

---

## Verification evidence

- **ctest macOS: 317/317** · **ctest Linux (Ubuntu 24.04 / GCC 13.3, Docker): 324/324** (Linux carries +7
  `EncryptedFileSecureStore` tests). All 10 merge tests confirmed passing under real GCC.
- Both C symbols `_scrivi_merge_scene` and `_scrivi_merge_chapter` confirmed **exported** in
  `libScriviCore.a` via `nm`, so the Apple app (which links the prebuilt archive) could call them in
  SP-075.
- Boundary stayed pure **C ABI / JSON-over-`std::string`** — no struct interop, no pbxproj change.
- ✅ **App adoption landed in SP-075** (`[Apple]`, closed 2026-07-21) and **SP-076** (`[Linux]` parity,
  closed 2026-07-22) — the condition SP-074's retrospective set before these could be considered fully
  verified. **EP-028 then closed 2026-07-22, user-approved.**

**Related:** **I-0083** (chapter-merge data loss — the defect T-0300 fixes), I-0084, T-0302/T-0303
(SP-075 Apple adoption), T-0304–T-0309 (SP-076 Linux parity).

---

*Verified by user ruling 2026-08-19; archived the same day during audit remediation. ⚠️ The delay between
implementation (2026-07-21) and this record is itself the finding — see the audit remediation log.*
