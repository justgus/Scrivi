# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-100 | EP-031 verification & Epic close (⚠️ **owns the AC1 re-verification**) | EP-031 | 🔵 Planning — **plan complete 2026-08-19, NOT activated** (**runs last**) |

**SP-100 is the only Sprint in Planning** — and the last of EP-031's eleven.

> ✅ **SP-100 PLANNED 2026-08-19.** ⚠️ Its full plan is written into
> [`Sprint-active.md`](Sprint-active.md) **while it is still 🔵 Planning** — that file carries a banner
> saying so. It moves to Active only on **direct user approval**; the row above stays here until then.
>
> **Three Tasks: T-0390 → T-0418 → T-0391**, in that order (each is evidence for the next; T-0391 reads
> all of them and cannot start early). **T-0418 is new at planning.**
>
> **Four rulings, all user-approved:**
> **R1** — ⚠️ **AC1's *"legacy 5-kind files load unchanged"* clause is STRUCK** and AC1 amended to the
> 11-kind ruled-scope round-trip. The clause contradicts the Epic's own §3.0 no-migration ruling;
> re-verifying it would assert a behaviour EP-031 deliberately chose not to build.
> **R2** — ⚠️ **AC10's *"existing projects open unchanged"* clause is STRUCK** (vacuous — Scrivi has not
> shipped) and AC10 amended to require green suites on **both architectures + sanitizers**. **Stronger
> than the original**, and only possible because SP-106 restored the x86-64 gate.
> **R3** — T-0390 **documents AND tests**; doc/code disagreements are **filed as Issues, not fixed**. A
> verification sprint that writes fixes stops being one (SP-099's R4 is the precedent).
> **R4** — ⚠️ **the live-use pass is required evidence, not polish.** Four of eleven sprints were
> unplanned, every one from USE; SP-102's live runs produced eight Issues, one of which (**I-0129**) *was*
> the AC23 clause itself and was invisible to every fixture.
>
> ⚠️ **Both amended ACs remain UNTICKED** — amending is not verifying. **Claude cannot close EP-031.**

---

⚠️ **No closure notes are kept in this file.** A Sprint leaves the backlog at **activation** and never
returns; whether it later closed is recorded in [`Closed/`](Closed/) and in
[`Sprint-Documentation.md`](Sprint-Documentation.md)'s All-Sprints table.

> **Removed 2026-08-19 under audit ruling R-28:** ~230 lines of closure records for **SP-052 → SP-106**.
> Every one duplicated its `Closed/Sprint-SP-XXX.md` archive, and the file had accreted so far that its
> single Planning row sat under a year of history.
>
> ⚠️ **Before deletion, the planning rulings inside those notes were checked against their archives**
> (the R-28 safeguard) — SP-095's gating ruling, SP-096's ID-prefix finding, SP-103's scope ruling and
> T-0410's OBE removal, and the SP-097/098/099/102/105/106 rulings. **All present in `Closed/`.
> Nothing was reconstructed because nothing was lost.**

*Last Updated: 2026-08-19 (audit remediation — ruling R-28).*
