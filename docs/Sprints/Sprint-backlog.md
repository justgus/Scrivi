# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-098 | **Integrity: prune, orphans, promotion, ⚠️ pending-vs-dangling** (⇄ swapped) | EP-031 | 🔵 Planning |
| SP-099 | Worldbuilding-object cards | EP-031 | 🔵 Planning |
| SP-100 | EP-031 verification & Epic close | EP-031 | 🔵 Planning |

> ✅ **SP-092, SP-093, SP-094, SP-101 all closed 2026-08-11 (Human-approved) — double Epic close.** Their rows
> and Planning blocks were removed from this file in the same step (per the standing rule that closed sprints
> leave the backlog). Archives: `Closed/Sprint-SP-092.md`, `-SP-093.md`, `-SP-094.md`, `-SP-101.md`.
> **SP-057** was superseded into SP-094 and is likewise removed; its record lives in `Closed/Sprint-SP-094.md`
> and `../Epics/Closed/Epic-EP-019.md`.
>
> **EP-019 ✅ CLOSED** and **EP-030 ✅ CLOSED** (both 2026-08-11) → `../Epics/Closed/`.
> EP-031's ScriviCore sprints (SP-095–SP-098) have no dependency on EP-030; only SP-099 needs the card framework.

> ✅ **SP-097 closed 2026-08-12 (Human-approved)** — `Closed/Sprint-SP-097.md`. ⚠️ **SP-097 and SP-098 had
> their CONTENT SWAPPED** at planning: SP-097 became **world packages** (`.scrivworld`, bindings,
> `worldID`-verified resolution, write locking, epoch chain — **T-0381–T-0385 plus T-0403 and T-0404**, all
> Verified), and the whole integrity set moved to **SP-098**. Two integrity tasks were verified unbuildable
> without worlds: `promote_object` had no destination while `artifact` was gated, and pending-vs-dangling had
> no world plumbing to interrogate. **Sprint IDs stayed in sequence — only the content order changed.**
> All three deferred items landed: the **`rule` relocation** (from SP-095), the **faction↔faction test**
> (from SP-096 — **AC3's last clause, now closed**), and **AC1's three gated kinds**.
> ctest **477/477 macOS** + **484/484 Linux (GCC 14)**, interop **59 passed / 0 failed**;
> ⚠️ **10 additive `scrivi.h` endpoints**. **EP-031 AC3 + AC6 + AC8 met.** **Next up: SP-098.**

> ✅ **SP-096 closed 2026-08-12 (Human-approved)** — `Closed/Sprint-SP-096.md`. EP-031's second sprint and the
> Epic's core deliverable: relation types, `relationships.jsonl` append-log, canonical edges + duplicate
> rejection, compaction — **T-0373–T-0376 plus T-0402**, all Verified. ctest **455/455 macOS** +
> **462/462 Linux (GCC 14)**, interop **59 passed / 0 failed**. ⚠️ **First `scrivi.h` change since EP-029** —
> 5 additive endpoints. **EP-031 AC5 met; AC3 met but for its faction↔faction clause** (needs SP-098).
> Its Planning row was removed above at close, per the standing rule. **Next up: SP-097.**
>
> The three planning rulings all held: **R1** Doc 1 §5.2's endpoint **ID-prefix rule was verified broken**
> against the shipped generators and was replaced by index-lookup resolution (T-0402 also amended the design);
> **R2** all four §5.1 relation types seeded, scene-endpoint ones included; **R3** `scrivi_list_edges_for`
> shipped here, giving the sprint a complete, directly-testable read path.

> ✅ **SP-095 closed 2026-08-12 (Human-approved)** — `Closed/Sprint-SP-095.md`. EP-031's first sprint:
> object kinds + `WorldObjectFields` extensions + `objects/index.json`; T-0370/T-0371/T-0372/T-0401 all
> Verified. ctest **432/432 macOS** + **439/439 Linux (GCC 14)**, interop **59 passed / 0 failed**.
> **EP-031 AC2 fully met.** Its Planning row was removed from the table above at close, per the standing rule.
> **Next up: SP-096.**
>
> Scope was fixed by three user rulings, all honoured: world-scoped kinds **declared but gated** until SP-098
> (no file landed where it must later be moved from — Doc 3 §7 writes no migration code); **`source`/T-0365
> held out entirely**, deferred with no sprint pending **OQ-1**; and **T-0401 added** for index
> rebuild/corruption coverage.
>
> ⚠️ ~~**Carried into SP-098:** relocating `rule` to world scope + the Package Structure §11 correction.~~
> **Done in SP-097 (T-0404)** after the content swap — `rule` is world-scoped and §11 is corrected.

> ✅ **SP-056 archive written 2026-08-05** — `Closed/Sprint-SP-056.md`, reconstructed from the EP-019 record
> (the sprint closed 2026-07-27 user-approved but was never archived; `Closed/` had jumped SP-055 → SP-058). The
> archive is marked as reconstructed. Its tasks T-0213/T-0214 were also still showing 🔵 Backlog in
> `Task-backlog.md` despite being Verified — corrected the same day.

> ✅ **SP-091 closed 2026-08-05 (Human-approved)** — `Closed/Sprint-SP-091.md`. Writing-tool cards + the
> `scrivi.scene.v1` extension; T-0392/T-0393/T-0363/T-0364 Verified, I-0101 fixed in-sprint. **SP-092 is next.**

> ✅ **SP-090 closed 2026-08-05 (Human-approved)** — `Closed/Sprint-SP-090.md`. EP-030's card framework
> shipped; T-0359–T-0362 all Verified. Planning row removed. **SP-091 is next.**

> ⚪ **Sequencing note (2026-08-09) — fully executed, retained for the record.** It read: EP-030 SP-090 ✅ →
> SP-091 ✅ → SP-092 → **SP-093** (EP-019 history capture) → **SP-094** (merged verification & double Epic
> close) → EP-031 SP-095→SP-100. **All of it happened as planned**, plus one unplanned sprint: **SP-101**,
> opened when SP-094's pass found EP-030 AC12 unimplemented. EP-019 and EP-030 both closed 2026-08-11.

> **SP-078 activated & ✅ closed 2026-07-22** — `Closed/Sprint-SP-078.md` (EP-024 `[Linux]` Scene Inspector
> Panel, first & only sprint; tasks T-0318–T-0320 Verified live over VNC). **This closed EP-024** in one sprint,
> mirroring Apple EP-014/SP-037. SP-078 was defined in the Epic (not staged here), so no Planning entry existed
> to remove.

> **SP-073 activated 2026-07-19** — moved to `Sprint-active.md` (EP-023 `[Linux]` chapter drag-reorder +
> Epic verify/close; tasks T-0294–T-0297).
> **SP-074 activated 2026-07-20, ✅ closed 2026-07-21** — `Closed/Sprint-SP-074.md` (EP-028 `[ScriviCore]`
> merge endpoints + I-0083 core fix; ctest macOS 317/317 + Linux 324/324). **SP-075 `[Apple]` activated
> 2026-07-21, ✅ closed 2026-07-21** — `Closed/Sprint-SP-075.md` (adopt the endpoints on macOS; T-0302/T-0303).
> **SP-076 `[Linux]` ✅ closed 2026-07-22** — `Closed/Sprint-SP-076.md` (scene & chapter merge parity;
> T-0304–T-0309). **SP-077 `[Linux]` ✅ closed 2026-07-22** — `Closed/Sprint-SP-077.md` (native menu bar;
> T-0310–T-0317). **This closed EP-028.** SP-074–SP-077 were all defined in the Epic, not staged here, so no
> Planning entry existed to remove.

Design reference for all: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06).
Epic-level ACs: `docs/Epics/Epic-active.md` (EP-019). Task detail: `docs/Tasks/Task-backlog.md` (T-0198–T-0217).

> **SP-051 activated 2026-07-06** (design signed off at activation) — moved to `Sprint-active.md`.
> **SP-052 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-052.md`.
> **SP-053 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-053.md` (EP-019 AC1 delivered & verified).
> **SP-054 activated 2026-07-07**, ✅ **closed 2026-07-09** — `Closed/Sprint-SP-054.md` (EP-019 AC3 + AC5 delivered & verified).
> **SP-055 activated 2026-07-10**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-055.md` (EP-019 AC4 + AC5's deferred branch clauses delivered & verified).
> **SP-056/SP-057 parked 2026-07-13** — EP-019 `[Apple]` was deferred to the Epic backlog; these sprints remain 🔵 Planning but are **not** next-up. The active line of work is the `[Linux]` app family (EP-020+).
> **SP-058 activated 2026-07-13**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-058.md` (EP-020 `[Linux]` App Foundation delivered & verified; EP-020 → Complete). Next `[Linux]` sprint is defined when EP-021 is drafted.
> **SP-059 activated 2026-07-13**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-059.md` (EP-021 `[Linux]` Project Lifecycle create-half: appSupportRoot + recents + landing + create-project; AC1/AC2/AC4/AC5 delivered & user-verified over VNC; T-0229 `EncryptedFileSecureStore` added mid-sprint).
> **SP-060 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-060.md` (EP-021 `[Linux]` open/close half: Open Project + 3 open modes + close→landing + full-loop smoke; AC3/AC6/AC7/AC8 delivered & VNC-verified). **This closed EP-021.** Next `[Linux]` Epic **EP-022** (Writing Surface) is Active; its first sprint (SP-061) is drafted at planning.
> **SP-061 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-061.md` (EP-022 `[Linux]` shell flip to Widgets host + scene navigator + read-only continuous viewport; AC1 + AC2 delivered & user-verified over VNC; `QPlainTextDocumentLayout` render bug found in click-through and fixed).
> **SP-062 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-062.md` (EP-022 `[Linux]` editable viewport + per-scene auto-save (debounce+switch+close/quit) + in-editor `Ctrl+Return`/`Ctrl+Shift+Return` scene/chapter creation; **AC3** delivered & user-verified over VNC; T-0238–T-0242 Verified. I-0061 Quit-button regression fixed+verified; I-0062 live chapter-label deferred to EP-023). Next EP-022 sprint is **SP-063** (scroll-driven switching).
> **SP-063 activated 2026-07-15**, ✅ **closed 2026-07-15** — `Closed/Sprint-SP-063.md` (EP-022 `[Linux]` scroll-driven scene switching + navigator↔scroll sync; **AC1/AC4 scroll portion** delivered & user-verified over VNC; T-0243–T-0245 Verified. Navigator-click requirement reversed mid-sprint: click takes the **caret to the clicked scene's start** (not caret-free) per user. T-0249 logged unscheduled — Page Fwd/Back + jump to manuscript start/end, bindings TBD). Next EP-022 sprint **SP-064** (cursor/focus + restore + verify, closes the Epic).
> **SP-064 activated 2026-07-15**, ✅ **closed 2026-07-15** — `Closed/Sprint-SP-064.md` (EP-022 `[Linux]` cursor/focus polish (focus-on-open, caret normalization out of boundary gaps, non-deletable separator) + quit→reopen **surface restore** (last active scene + cursor + scroll via the `restored{}` payload + `save_scene` selection/scroll args — filled the 0/0/0.0 stub) + full EP-022 verify; **AC4-cursor/AC5/AC6/AC7** delivered & user-verified over VNC; T-0246–T-0248 Verified; no ScriviCore work, `scrivi.h` untouched). **This closed EP-022.** Next `[Linux]` Epic is **EP-023** (structure editing), drafted when the user activates it.


*Last Updated: 2026-08-09 (**Tracking-doc audit — backlog realigned to the 2026-08-07 renumbering.** This file
was the main source of the SP-093 confusion: its summary table still read "SP-093 | EP-030 verification & Epic
close" while the struck-through detail entry 130 lines below said the opposite. Corrected: **SP-093** is the
`[Cross]` **EP-019 history-capture sprint** (detail entry added); **SP-094** is the merged **"EP-019 + EP-030
verification & Epic close"** (detail entry added); **SP-057** marked ⚪ Superseded → SP-094 rather than
"parked"; **SP-092** status corrected 🔵 Planning → 🟡 Active; **EP-031 renumbered SP-094–SP-099 →
SP-095–SP-100** to clear the collision with the merged verification sprint (`Epic-active.md` and its 22
T-0370–T-0391 rows realigned to match, per `Sprint-active.md:357`); the 2026-08-05 sequencing note superseded.
Prior note follows.)*

*2026-07-22 (**SP-078 planned, activated, and implemented same day** — opened EP-024 `[Linux]`
Scene Inspector Panel (mirrors Apple EP-014); moved to `Sprint-active.md`. Default visibility = **shown**
(Apple parity, user decision). Container build green (184/184, 0 warnings) + all Linux smokes PASS; live VNC
verify + AC sign-off pending. SP-056–SP-057 remain 🔵 Planning, parked behind deferred EP-019 `[Apple]`.)*
