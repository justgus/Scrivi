# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

| ID | Title | Sprint | Epic | Implemented |
| -- | ----- | ------ | ---- | ----------- |
_No Tasks awaiting verification._

⚠️ **SP-122's T-0466–T-0471 were ✅ Verified 2026-08-25** and archived to
[`Verified/Task-verified-0466-0471.md`](Verified/Task-verified-0466-0471.md) in the same step SP-122 closed.

⚠️ **SP-121's T-0460–T-0465 were ✅ Verified 2026-08-25** and archived to
[`Verified/Task-verified-0460-0465.md`](Verified/Task-verified-0460-0465.md) in the same step SP-121 closed.

> ⚠️ **T-0365 must not be Verified on the card alone.** It renders correctly and shows *"No sources cited
> by this scene's objects"* — which is indistinguishable from working, because there is no way to create
> a source to test it with. Its write half was ✅ **PAID by SP-120** under [EP-034](../Epics/Closed/Epic-EP-034.md), ✅ **now CLOSED 2026-08-25**, and §3.1.1's
> object-card entry point to `CitationPopover` is built but unwired.

---

*Last Updated: 2026-08-25, fourth pass (**no Tasks awaiting verification** — SP-122's six ✅ Verified and
archived the same day. Prior note follows.)*

*Last Updated: 2026-08-25, third pass (**six Tasks awaiting verification** — SP-122's **T-0466–T-0471**;
✅ **AC12 MET**. Prior note follows.)*

*Last Updated: 2026-08-25, second pass (**no Tasks awaiting verification** — SP-121's six were
✅ Verified and archived the same day. Prior note follows.)*

*Last Updated: 2026-08-25 (**six Tasks awaiting verification** — SP-121's **T-0460–T-0465**, moved here
from `Task-active.md` per `Task-Guidelines.md` §"When Marking Task as Implemented - Not Verified".
Prior note follows.)*

*Last Updated: 2026-08-20 (**no Tasks awaiting verification** — SP-115's seven were Verified and archived
the same day. Corrected a stale link: **EP-034 is now 🟡 Active**, not in the backlog. Prior note follows.)*

*Last Updated: 2026-08-19 (audit remediation — rulings R-16, R-17).*

| **T-0521** | ✅ **Remove `rebuildStorage`'s O(N²) chapter lookup** — ✅ **[I-0196]'s third defect.** ⚠️ **AT EVERY CHAPTER BOUNDARY it runs `allScenes.first(where:)` (1,179 scenes) AND rebuilds the ENTIRE `chapterID → ordinal` map from scratch via `allScenes.map(\.chapterID)`** (`ManuscriptTextView.swift`, inside `rebuildStorage`'s per-segment loop) — ⚠️ **~55 chapters x 1,179 scenes ≈ 130,000 ops per rebuild.** ✅ **FIX: precompute chapter titles and ordinals ONCE per rebuild.** | ⚠️ **SP-132 (closed)** | ⛔ **NOT STARTED — ⚠️ CORRECTED 2026-09-15.** ⚠️ **The row said `Implemented - Not Verified`; ✅ READING THE CODE SHOWS IT IS NOT IMPLEMENTED — both scans are still present, unchanged.** ⚠️ **I moved this task on the strength of a stale status line WITHOUT checking the code, the same failure the user caught on [T-0529].** ✅ **Verify status against the CODE, not the row.** ---- ⚠️ **NAMING COLLISION — ✅ THE REASON THIS GOT CONFUSED, RECORDED SO IT DOES NOT RECUR.** ⚠️ **THERE ARE TWO \"O(N²) CHAPTER LOOKUPS\" AND I CALLED BOTH BY THE SAME NAME:** ✅ **(A) `ViewportSceneLoader.renumberChapterTitlesFrom` (`:794`) — O(chapters × scenes) with one `@Observable` write per scene. ✅ FIXED 2026-09-15 as part of [I-0213]; it helped take `createChapter WORK` from `880 ms` → `305 ms`.** ⛔ **(B) THIS TASK: `ManuscriptTextView.rebuildStorage`'s heading block (`:629`) — `allScenes.first(where:)` PLUS a full `allScenes.map(\.chapterID)` ordinal rebuild AT EVERY CHAPTER BOUNDARY. ⛔ NOT FIXED.** ⚠️ **Both are real; ✅ only (A) is done.** ---- ✅ **CURRENT IMPACT IS NIL AND THAT IS MEASURED:** ⚠️ **`rebuildStorage`'s own loop reports `0.0 s` on 1,179 segments in every 2026-09-15 run** — ✅ **TextKit 2 made the storage build cheap enough that ~130,000 operations vanish into the noise.** ⚠️ **This is a LATENT O(N²) that would bite a much larger manuscript, ⛔ NOT a live defect.** |
