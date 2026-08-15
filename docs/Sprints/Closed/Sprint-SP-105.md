# SP-105 — `[Cross]` World search indexing (I-0118)

**Status:** ✅ **CLOSED 2026-08-15 (user-approved).** I-0118 ✅ Resolved - Verified.
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Codebases:** `[ScriviCore]` + `[Apple]`
**Opened:** 2026-08-14, immediately after SP-104, to implement the I-0118 ruling.

---

## Goal

Restore search reach lost to T-0409. Since every worldbuilding kind became world-scoped and only
`objects/` was ever indexed, **no character, location or item was findable in Spotlight** — a regression
against pre-ruling behavior, not a settled design.

Implements the four rulings recorded in `Issue-active.md` under **I-0118**.

## Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0118 | ⚠️ World contents are not Spotlight-indexed — a regression in reach | Medium | ✅ **Resolved - Verified (2026-08-14, user-approved)** |

---

## What shipped

**`[ScriviCore]`**

- `collectObjects` takes a **base directory**, a deep-link ownership clause and a domain, instead of
  hardcoding `<project>/objects` and the project. It no longer knows which side it is scanning.
- `extractSearchableText` resolves bound worlds via `WorldStore` and scans **available** ones.
- New `collectWorldExtras` indexes the non-`ObjectKind` package contents (**Q4**): `historical-events`,
  `historical-timelines`, `assets`. Deliberately schema-light and best-effort — one malformed file costs
  only itself, matching `collectObjects`.
- `SearchableItem` gains a per-item `domainIdentifier`; `ExtractSearchableTextResult` gains
  `worldDomainIdentifiers`. **Empty per-item domain means "the project's"**, so every existing
  project-side record is byte-identical to before and no call site had to change.
- ⚠️ **The C ABI serializer was widened in the same step** (`scrivi_c_api.cpp`). New fields in C++ that
  the boundary drops are invisible to every facade test — that is exactly how I-0113 shipped.

**`[Apple]`**

- `SpotlightDonor.donate` uses each record's own domain when it names one.
- `SpotlightDonor.deleteProject` **refuses a world domain outright** and logs it. Not defensive noise:
  it runs on every project close, so a world domain reaching it would silently destroy shared search
  data with no user action and nothing to notice.
- `ScriviDeepLink` parses the `world=` form (**Q2**). Previously it required `project=`, so a world link
  returned `nil` and a tapped Spotlight hit for a character **did nothing at all**.
- `handleDeepLink` routes world links: one open binding project → focus it; zero or several → say plainly
  what is missing. Opening a world in its own right is **EP-033's** question, not improvised here.
- `SearchableItemResult` / `SearchableContentResult` decode the new fields.

---

## Verification

**Probed against the user's real project** through `scrivi_*`, not the facade:

```
projectDomain : project_019fa3be-dac9-7f2d-b2dd-23dbb3291a9e
worldDomains  : ['world_character_01a000fb-539a-7402-802e-0d97eeb1e594']
world items   : 4   (Petch, Myton, Veyra + 1 location)
sample link   : scrivi://open?world=world_character_01a000fb-…&item=character:character_01a00120-…
```

⚠️ **A defect the probe caught that no test would have.** The domain first came out as
`world_world_<uuid>` — `worldID` already carries a `world_` prefix (`WorldStore.cpp:101`) and the code
added another. Cosmetic, and every test passed with it, because the tests asserted the same expression
the code computed. It would have been **stamped into every donated Spotlight entry** and far cheaper to
fix before any were in the index than after. Domains are now bare IDs; `project_…` and `world_…` remain
disjoint without a synthetic prefix.

**Tests:** ctest **516 / 516** (513 → 516; three added).

- `SearchableContentTests` — the two I-0118 guard cases **flipped from asserting zero to asserting
  reach**. They were written in SP-104 to fail loudly when indexing landed, and they did.
- New: an **unavailable world is skipped, never pruned** (Q3) — asserts the world contributes no domain,
  so the donor is never even told about one it might clear.
- New: **the whole package is indexed** (Q4), including a malformed-file best-effort check.
- New (**C ABI**): world items carry their own domain and a world deep link, with `source` as the
  control proving the project/world halves did not collapse into one.

✅ **macOS interop 86/86 — RESOLVED 2026-08-14.** The suite was briefly unrunnable
(`Could not launch "ScriviInteropTests"`, LaunchServices error 20); the cause was a **running Scrivi
instance holding the app bundle**, not this work — it reproduced on a stashed clean tree. Once the user
quit the app the suite ran.
⚠️ **And it immediately found 15 stale tests** — every one a `worldRequired` refusal or an assertion
encoding the **pre-T-0409 partition**, including two asserting the old four-kind world scope. They had
been silently stale for two days while the suite could not run.

---

## Not done

- **Removal of world entries.** Under the Q1 ruling nothing deletes them, and no affordance exists —
  deferred to **EP-033** (in-app view vs. dedicated world-management app). Until then a world's entries
  are **write-only** from the writer's point of view. This is the ruling working as intended, but it
  should be stated plainly rather than discovered.
- **Linux** — the Qt app donates nothing to any search index, so the `[Apple]` half has no counterpart.
  The `[ScriviCore]` half is cross-platform and covered by ctest.
- **Spotlight end-to-end confirmation** — that entries actually appear in a macOS search. The donor
  reports "accepted for processing", which is explicitly **not** a guarantee the index took them
  (see the note at `SpotlightDonor.swift:57`).

---

## Retrospective (closed 2026-08-15, user-approved)

**Restored search reach that T-0409 had silently removed.** Since the scope ruling, no character, location
or item was findable in Spotlight — a regression the test suite was actively *concealing*, because two
SearchableContentTests cases had been failing against the old expectations and read as "realignment work."

⚠️ **The design questions were answered before any code.** The user's Q1 ruling — *entries persist, never
reference-counted, never deleted unless expressly instructed* — **removed** the refcounting/unbind-hook
subsystem the problem appeared to require. The lifecycle problem does not exist if nothing deletes as a
side effect. Implementing before ruling would have built that subsystem needlessly.

⚠️ **A probe on the real project caught what every test missed:** the domain came out `world_world_<uuid>`,
because the tests asserted the same expression the code computed. It would have been stamped into every
donated Spotlight entry. Tests that mirror the implementation cannot catch the implementation.

**At close:** ctest **516/516**, macOS interop **86/86**, app **BUILD SUCCEEDED**. Removal of world entries
remains deferred to **EP-033** — world entries are write-only until that Epic rules.

*Last Updated: 2026-08-15.*
