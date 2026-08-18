# T-0389, T-0415 — Pending Presentation, Warning View, AC24 + the AC23 Live Run (EP-031, SP-102)

**Status:** ✅ **Both Verified 2026-08-17** (user-approved)
**Epic:** EP-031 — `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Sprint:** SP-102 · **Record of truth:** [`Sprint-active.md`](../../Sprints/Sprint-active.md)
**Delivers:** EP-031 **AC23** and **AC24** — the last two clauses of AC9.

---

## T-0389 — Pending footer + warning view + `offline`/`unmounted` refinement

✅ **Verified 2026-08-17.**

**1. Card-level §7.2 footer** — `PendingWorldFooter` (`ObjectCard.swift:723`). Groups pending entries
**by world**, names each world, states its status, and says the links are **held pending**. `Entry`
gained `pendingWorldID` so the card can name the world rather than warn anonymously. Before this, the
world and its status appeared **only in a hover tooltip** — undiscoverable, and absent on iPad.

**2. Warning view** — `Scrivi/Views/WorldWarningView.swift` (`WorldWarningModel` + `WorldWarningView`),
below the manuscript and above the Timeline, with `worldWarningVisible` on `ProjectSession` and a
**"Show World Warnings"** toggle at all four View-menu sites plus the iOS toolbar menu. Reloads on
appear **and** on app-foreground — ⚠️ a drive can be ejected *while the project is open*, which is the
whole AC23 scenario. Reports **every unavailable bound world**, not only those with pending edges: a
world that is away with nothing yet linked still blocks creation, and silence would read as "all is
well". ✅ **No destructive affordance** — it routes to Manage Worlds and performs no repair.

**3. AC24 refinement** — `Scrivi/App/WorldVolumeStatus.swift`, applied at **`WorldEntry.worldStatus`**,
the single accessor all five call sites already read, so no site restates the rule (the kind-list
defect in another costume).

> ⚠️ **Proven RED before green, and it caught the real defect.** Mutating `WorldVolumeStatus` to the
> *natural* rule — key `unmounted` off `volumeIsRemovableKey`/`volumeIsEjectableKey` — fails with
> `Expectation failed: (status → .missing) == .unmounted`. **That is I-0115 recurring:** the user's own
> Eskandar drive reports **both flags `false`**, so the natural implementation would tell him his world
> was *missing* — inviting a destructive remedy — when the drive was merely unplugged. The shipped rule
> uses **volume-root mount presence** instead, degrading to `unavailable` and **never guessing
> `missing`**.

## T-0415 — AC23 live verification on the real USB world rig

✅ **Verified 2026-08-17.** ⚠️ **Run twice** — [[I-0128]] records itself as found *"in the T-0415
re-run"*.

AC23 is not "pending rows render"; it is *"reattaching restores the card **with no writer
intervention**"*. It was split out of T-0389 at SP-102 planning precisely because **a fixture cannot
establish that** — burying it inside an implementation task is how it gets reported done against a
fixture.

**Both branches covered on the real rig** (tintagael project + Eskandar world on USB):

- **`unmounted` → reattach** — the no-intervention clause, which is exactly what **failed and was
  fixed**: see I-0129 below.
- **`missing` → relink** via "Locate…" — see I-0130.

---

## ⚠️ What the live runs actually produced — eight Issues, all Verified the same day

**This is the AC23/AC24 evidence.** The runs were not a formality; they found eight real defects, every
one user-approved Verified on 2026-08-17 →
[`Issue-verified-0121-0130.md`](../../Issues/Verified/Issue-verified-0121-0130.md).

| Issue | What the live run exposed |
| ----- | ------------------------- |
| **I-0123** | A world reappearing **while Scrivi runs** was never re-activated — blind until relaunch |
| **I-0124** | While a world was away, **every pending object appeared on every world-scoped card** — locations and chronicles listed under Characters |
| **I-0125** | Creating any object but character/location **failed at the edge step**, leaving the object orphaned |
| **I-0126** | A card's error message survived draft dismissal until the scene changed |
| **I-0127** | The "Add ⟨Kind⟩…" picker popover was too short to use |
| **I-0128** | Inspector cards **did not refresh on reconnect** — the writer had to change scenes |
| **I-0129** | ⚠️ **Availability refreshed on app FOCUS, not on the drive mounting.** Found by reversing the usual order — return to Scrivi *first*, then plug the drive in — so focus never changed and the warning stayed up. **This is the AC23 no-intervention clause itself.** Now driven by `NSWorkspace.didMount/didUnmountNotification`, proven with a `hdiutil` probe |
| **I-0130** | "Locate…" repaired the world but the project-wide warning stayed stale until the scene changed |

> ⚠️ **I-0129 is why AC23 needed a live run and not a fixture.** Ejecting a drive normally *requires*
> leaving the app, so focus always changed and the refresh always happened to fire — the defect was
> completely masked until the user deliberately reversed the order. No fixture would have caught it.

---

⚠️ *Archived 2026-08-18. **These two Tasks were carried as 🟠 Implemented-Not-Verified / 🔵 Planned for a
day after they were verified**, because SP-102's exit-criteria table was written at planning on
2026-08-17 — **before** that day's live runs — and was never updated afterward. Compounding it,
`Issue-verified-0121-0130.md`'s summary table listed only 3 of its 10 Issues, so I-0123–I-0129 (the
evidence) were invisible to anyone reading the index. Both files corrected in the same pass. The
user's challenge — "What is the pending footer/warning view verification. I thought that was already
verified. It shows." — was correct on every point.*
