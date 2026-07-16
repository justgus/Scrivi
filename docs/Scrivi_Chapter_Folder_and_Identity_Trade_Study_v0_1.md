# Scrivi — Chapter Folder Naming & Identity Source-of-Truth Trade Study (v0.1)

**Status:** ✅ **DECIDED (2026-07-16, Human-approved).** Adopt **A4b (fractional order-key slugs) + B3 (disk is
the source of truth for chapter identity AND order; the index `manuscript.meta.json` becomes a rebuildable
cache) + Order Authority = filesystem.** Additional approved scope: **(1)** add crash-safe **rename/move
primitives to ScriviCore's FileSystem port**; **(2)** **detect old-format projects and migrate them** to the new
scheme. Implementation is a new `[ScriviCore]` Epic (cross-platform — changes Apple packages too). See §7 for the
decision record and the answers to §6's open questions.
**Date:** 2026-07-16
**Author:** Claude (at the Human's direction)
**Related:** **I-0072** (chapter-folder slug collision corrupts `manuscript.meta.json`), I-0063/I-0064/I-0069/
I-0070 (chapter split/renumber), I-0065/I-0066 (a project whose index/history diverged from disk).
**Supersedes / touches:** on-disk layout in `Scrivi_Project_Package_Structure_v0_1.md`, schemas in
`Scrivi_Minimum_Schema_Set_v0_1.md`, repair behavior in `Scrivi_External_Change_Repair_Matrix_v0_2.md`.

---

## 0. Why this study exists

I-0072 root-caused a data-corruption bug: `ChapterCreator` names a new chapter's on-disk directory
`chapter-<chapters.size()+1>`. After any chapter delete leaves a gap, `count+1` collides with an existing
directory; the create then **overwrites the colliding chapter's sidecar** and **duplicates its index entry**.

The quick fix (make the slug collision-free) forces a naming-scheme decision. In examining that, the Human
surfaced a second, deeper question: the chapter's identity (`chapterID`) is **stored in two files** — the index
(`manuscript.meta.json`) and the sidecar (`chapter.meta.json`) — with no declared source of truth. The two
questions are coupled (both concern how the filesystem, the index, and identity relate), so they are studied
together but decided separately.

**Decision A — Chapter folder naming scheme.** How should `chapter-*` directories be named?
**Decision B — Identity/order source of truth.** Where does `chapterID` (and chapter order) authoritatively live?

The Human's stated goals, captured verbatim as design drivers:
- *"I like being able to look at the directory structure and see the manuscript in order"* (filesystem legibility).
- *"It would allow an easier repair mechanism if corruption of the index occurred"* (self-describing disk = recoverable).
- Concern that *renaming all the folders may require multiple disk operations and multiple file updates* (churn cost).
- *"The chapter id lives in one place"* — eliminate the duplication; candidate: drop `chapterID` from the index
  since `path` already resolves to the sidecar that carries it.

---

## 1. Ground truth (how ScriviCore works today — verified in source, 2026-07-16)

Confirmed by reading the code, not assumption:

1. **Directories are stable-identity, never renamed.** `ChapterCreator.cpp:42` assigns the slug once. **No code
   anywhere renames a `chapter-NNN` directory**, and the FileSystem port (`Services.hpp`) exposes only
   `exists / createDirectories / readTextFile / atomicWriteTextFile / listDirectory / removeFile` — **there is no
   rename or move primitive.** (`ChapterDeleter` deletes via `std::filesystem::remove_all` directly, bypassing
   the port.)
2. **Order lives in the index, not the filesystem.** `ChapterReorderer.cpp` reorders the `chapters[]` array in
   `manuscript.meta.json`; directories don't move. So after a reorder the folder names no longer reflect reading
   order (observed: index listed `chapter-017` between `chapter-007` and `chapter-008`).
3. **Delete leaves a permanent gap.** `ChapterDeleter.cpp:62,73` erases the index entry and the directory; it does
   **not** renumber survivors. (Observed gaps on disk: 011, 014 missing.)
4. **The slug is positional (`count+1`) but the architecture is identity-based** — the contradiction that *is*
   I-0072.
5. **The id is duplicated.** `ChapterRef { chapterID; path; }` (index) and `ChapterMetaData { chapterID; title;
   slug; … }` (sidecar) both carry `chapterID`. The sidecar also carries a `slug` field (folder-name concept is
   already modeled separately from `chapterID`).
6. **14 sites read the index**, incl. `ProjectValidator` and `ExternalChangeScanner`, which already walk
   `chapters[]`, resolve paths, and **repair by keying on `chapterID` and rewriting `ref.path`**
   (`ExternalChangeScanner.cpp:93-94`). This is the existing integrity/repair machinery.

**Key implication:** today's design is *"stable-identity directories + index-owned order"* (model #2 in the
Human's framing), but with a positional *name*. Making the disk reflect order (the Human's goal) is a move
**toward model #1** and must be weighed against churn (fact 1: there's not even a rename primitive yet).

---

## 2. Decision A — Chapter folder naming scheme

### Evaluation criteria
| # | Criterion | Why it matters |
|---|-----------|----------------|
| C1 | **Collision-safety** | The I-0072 class must be impossible by construction, not by luck. |
| C2 | **Filesystem legibility (order visible)** | Human goal: read the folder listing and see manuscript order. |
| C3 | **Insert/delete/move churn (disk ops + file writes)** | Human concern: renaming many folders per edit is costly & risky. |
| C4 | **Repair-ability** | Can the index be rebuilt from disk alone if it's lost/corrupt? |
| C5 | **Cross-platform stability** | Apple + Linux + future read the same package; names shouldn't break refs. |
| C6 | **Crash/atomicity safety** | A rename storm interrupted mid-way must not corrupt the manuscript. |

### Options

**A1 — Positional renumber (true model #1).** On every insert/delete/move, rename all affected `chapter-NNN`
dirs so names always equal reading order.
- C2 ✅ perfect legibility. C4 ✅ order derivable from names.
- C1 ⚠️ safe only if renames are transactional. C3 ❌ up to N renames per edit; each dir rename is a filesystem
  op **plus** a `ref.path` rewrite for every moved chapter. C5 ❌ any external/bookmark reference to a folder
  path breaks on every edit. C6 ❌ **worst case**: needs a rename primitive (none exists) and a multi-dir
  transaction; an interrupted renumber leaves half the folders shifted. **This is the model that motivated
  moving away in the first place.**

**A2 — Monotonic never-reused counter.** Keep `chapter-NNN`, but N = (highest N ever used) + 1, from a persisted
high-water mark. Never reused, gaps accumulate.
- C1 ✅ collisions impossible. C3 ✅ zero renames ever (create only). C5 ✅ stable names. C6 ✅ single create.
- C2 ❌ names do **not** reflect order (gaps + reorder mean `chapter-020` can be chapter 3). C4 ⚠️ order NOT
  derivable from names — still index-dependent. Minimal change from today; fixes I-0072 but abandons the Human's
  legibility goal.

**A3 — Identity-derived slug (`chapter-<id-suffix>`).** Name = a slice of the unique `chapterID`
(e.g. `chapter-019e9cdd`). Pure model #2.
- C1 ✅ collision-impossible. C3 ✅ zero renames. C5 ✅ maximally stable. C6 ✅ single create. C4 ✅✅ **strongest
  repair**: the folder name *is* the id, so the index can be rebuilt from disk with zero ambiguity (no id needed
  inside the sidecar even).
- C2 ❌ names are opaque; listing shows no order at all. Directly opposes the Human's legibility goal.

**A4 — Sortable order-key label (the Human's "chapter-aardvark" idea, generalized).** Name each folder with a
**lexicographically sortable ordering key** that reflects position, chosen so that **inserts pick a new key
*between* neighbors without renaming anyone else**. Two concrete encodings:
- **(A4a) Word labels:** a curated ordered word list (aardvark < baboon < …); insert between "aardvark" and
  "baboon" → "aard… something". Human-friendly but the "between two adjacent words" case eventually forces a
  rename (words run out of gaps).
- **(A4b) Fractional / rebalancing rank key** (a.k.a. "LexoRank"/order-key): names like `chapter-a0`,
  `chapter-a0m`, `chapter-a1`. Inserting between `a0` and `a1` yields `a0m` — a **new key with no other rename**.
  Sorts lexicographically = reading order. Occasional global "rebalance" (rare) is the only multi-rename event.
- C1 ✅ keys are unique by construction. C2 ✅ **sorted listing = manuscript order** (the Human's goal, met
  without per-edit renumbering). C3 ✅ insert/reorder = **rename only the one folder that moved** (+ its
  `ref.path`), not the whole tail — the Human's exact insight. C4 ✅ order derivable from names (sort them). C5
  ⚠️ a *moved* chapter's folder does rename (its path ref updates — but that update is already required for the
  reorder). C6 ⚠️ a single-folder rename per move needs the missing rename primitive + atomicity, but it's **one**
  rename, not N.

### Cross-cutting requirement for A1/A4 (and good hygiene for all)
Both order-reflecting options need a **safe rename** the core currently lacks. Minimum addition to the FileSystem
port: `Result<void> renamePath(from, to)` (or copy-meta + atomic-write-new + remove-old, if we keep renames to a
single dir). Must be crash-atomic or crash-detectable (see §4 repair).

### Recommendation — A (folder naming): **A4b (fractional order-key), with A2 as the low-risk fallback.**
A4b is the only option that satisfies the Human's two hard goals (C2 legibility + C4 disk-repairability)
**without** paying model #1's churn (C3/C6): an insert or move renames **exactly one folder**, and a plain
`ls | sort` shows the manuscript in order. It needs a single new capability (a safe single-path rename) and an
order-key generator (well-trodden algorithm). **If the appetite for that machinery is low right now,** ship
**A2** first (it kills I-0072 immediately with near-zero change and zero rename risk) and adopt A4b as a
follow-on — A2→A4b is compatible (A2 names are valid, if unsorted, keys during migration).

A4a (words) is charming and human-nameable but strictly worse than A4b on C3 (word-space between neighbors runs
out, forcing renames) — recommend words only as an optional *display* affordance, never as the ordering key.

---

## 3. Decision B — Identity / order source of truth

Today `chapterID` is in **both** the index and the sidecar; order is in the index only. The Human proposes
removing `chapterID` from the index (path resolves to the sidecar which has the id). Let's evaluate what each
file should authoritatively own.

### The two facts that must have exactly one owner each
1. **Chapter identity** (`chapterID`) — currently duplicated (index + sidecar).
2. **Chapter order** — currently index-only (the `chapters[]` array order). *Not* duplicated today, but option
   A4/A1 would make it **derivable from folder names too**, creating a second potential source.

### Options

**B1 — Sidecar owns identity; index owns order (drop `chapterID` from the index).** `ChapterRef` becomes just
`{ path }` (an ordered list of paths); the id is read from each sidecar. The Human's proposal.
- ✅ Single owner for identity (no more index/sidecar id divergence — the *exact* I-0072 symptom class where the
  index said `019e942e` and the sidecar said `019e9cdd`). ✅ Smaller index.
- ⚠️ Every consumer that today matches on `ref.chapterID` (e.g. `SceneReorderer`, `ExternalChangeScanner:93`)
  must instead **open the sidecar to learn the id** — more reads, and it couples "which chapter" to "the file is
  readable." ⚠️ Repair that currently keys on the index id now must trust the sidecar.

**B2 — Index owns identity + order; sidecar's `chapterID` becomes advisory/derived (or dropped).** Invert B1:
the index is authoritative for both id and order; the sidecar stops storing its own id (or treats it as a cache).
- ✅ Single owner; all order/identity questions answered without opening every sidecar (fast). ✅ Matches how 14
  consumers already work (index-first).
- ❌ **Weakest for the Human's repair goal**: if the index is lost/corrupt, a sidecar with no id can't be
  re-associated except by folder position — brittle. ❌ Runs *against* "self-describing disk."

**B3 — Sidecar owns identity; order is derived from folder names (needs A1 or A4).** Combine B1 with an
order-reflecting naming scheme so the index's *order* also becomes redundant — the index degrades to an optional
**cache/manifest**, and the **filesystem (folder names + sidecars) is the source of truth for both id and
order**.
- ✅✅ **Strongest repair**: the entire manuscript structure (which chapters, in what order) is reconstructable
  from disk alone — the index can be *rebuilt* wholesale, which is exactly the Human's "easier repair mechanism."
  ✅ Single owner for identity (sidecar) and order (sorted folder names). 
- ⚠️ Only coherent if Decision A picks an order-reflecting scheme (A4b/A1); with A2/A3 the names don't encode
  order so B3 can't derive it. ⚠️ Makes the index a derived artifact — every writer must keep it in sync *or* we
  accept "rebuild on load," a bigger behavioral change (though very much in the spirit of I-0066's history
  self-heal).

### Recommendation — B (source of truth): **B1 now; evolve toward B3 if A4b is adopted.**
- **B1** directly eliminates the divergence class behind I-0072 (identity can no longer disagree between two
  files, because it's stored once). It's the smallest correct step and independent of Decision A. The added
  sidecar reads are modest and cache-able.
- **B3** is the *architecturally strongest* end-state for the Human's repair goal, but it is only unlocked by an
  order-reflecting naming scheme (A4b). So: adopt B1 immediately; if/when A4b lands, promote the index to a
  rebuildable cache (B3) and add a load-time "rebuild index from disk if inconsistent" repair (mirrors I-0066).
- **Reject B2** — it optimizes for read-speed at the cost of the Human's explicit repairability goal.

**Order:** whichever B option, **order stays index-authoritative until A4b exists.** Do not make order
disk-derived while folder names are still positional/opaque (A2/A3) — you'd have no reliable disk source for it.

---

## 4. Interaction, migration, and repair

- **Coupling summary:** A2/A3 (opaque names) pair only with B1/B2 (index-or-sidecar owns order). A4b/A1
  (order-in-names) *unlock* B3 (disk owns everything; index becomes a cache). The Human's dual goal (legible +
  repairable) is fully satisfied **only by A4b + B3**; A4b + B1 is a strong, lower-risk intermediate.
- **Migration:** existing packages have `chapter-NNN` dirs and `chapterID` in both files. Any target is
  reachable without a flag day: (1) new creates use the new slug scheme; (2) a one-time normalizer can rewrite
  the index (and, for B3, rebuild it from disk); (3) old `chapter-NNN` names remain valid keys (they sort, just
  with gaps) so no forced rename. A migration should be *lazy/idempotent* (repair-on-open), consistent with the
  External-Change-Repair matrix.
- **Repair (the payoff):** with A4b + B3, a lost/corrupt `manuscript.meta.json` is fully recoverable —
  `listDirectory` the `chapter-*` folders, sort by order-key, read each sidecar's id/title. Add this as a
  ScriviCore load-time self-heal (like I-0066's history prune). Even with B1 alone, the sidecar-owns-id rule lets
  a validator *detect* and drop the I-0072 phantom/duplicate entries (index id ≠ sidecar id ⇒ prune).
- **New primitive required (A4b/A1):** a crash-safe single-path rename in the FileSystem port. Spec it to be
  atomic or leave a detectable half-state the load-time repair can finish.

---

## 5. Consolidated recommendation

| Decision | Recommended | Rationale | Fallback |
|----------|-------------|-----------|----------|
| **A — folder naming** | **A4b — fractional order-key slug** | Only option meeting *both* legibility (C2) and disk-repair (C4) without model-#1 churn (C3/C6); insert/move renames one folder. | **A2 — monotonic counter** (ships the I-0072 fix immediately with ~zero risk; A2→A4b compatible). |
| **B — source of truth** | **B1 now → B3 once A4b lands** | B1 removes the id-duplication that *is* the I-0072 divergence; B3 (disk-authoritative, index-as-cache) is the strongest form of the Human's repair goal, unlocked by A4b. | — |
| **Order authority** | **Index, until A4b exists** | No reliable disk source for order under opaque names. | Disk-derived (B3) after A4b. |

**Minimum to close I-0072 safely if we want to move today:** A2 + B1 + a validator that prunes index entries
whose `path`'s sidecar id ≠ the index id (heals existing damage). This is small, low-risk, and non-committal —
it does **not** foreclose adopting A4b + B3 later.

**Biggest-value end-state:** A4b + B3 — the disk is self-describing and in order; the index is a rebuildable
cache; identity lives in exactly one place; inserts/moves rename at most one folder.

---

## 6. Open questions for the Human

1. **Appetite for the rename primitive now?** A4b needs a crash-safe single-path rename in the FileSystem port.
   Build it now (enables the end-state) or ship A2 first and defer?
2. **Index as cache (B3) vs. authority (B1)?** B3 is the strongest repair story but makes the index a derived
   artifact with a rebuild-on-open behavior — acceptable direction, or keep the index authoritative (B1)?
3. **Scene folders/slugs:** scenes have the *same* `NNN-slug` positional pattern inside a chapter (`SceneCreator`).
   Should this study's outcome apply symmetrically to scenes, or is that a separate follow-up?
4. **Human-readable labels:** keep an optional word/title hint in the folder name for eyeball use
   (`chapter-a0m--the-reckoning`), or keep folder names purely mechanical and rely on the sidecar `title`/
   `displayLabel`?
5. **Cross-platform sequencing:** this is `[ScriviCore]`, so it changes Apple's on-disk packages too. Do we want
   an Epic that lands core + both platforms together, or core-first behind the existing behavior with a migration?

---

## 7. Decision record (2026-07-16, Human-approved)

**Approved end-state:** **A4b + B3 + Order Authority = filesystem**, with **rename/move primitives** added to
ScriviCore and **old-format project migration**. No intermediate A2/B1 step — go straight to the end-state.

### 7.1 What this locks in
- **Folder names are fractional order-key slugs** (A4b): `chapter-<orderKey>`, where `orderKey` is a
  lexicographically-sortable rank string (e.g. `a0`, `a0m`, `a1`). `ls | sort` = manuscript reading order.
- **The filesystem is the source of truth** (B3): a chapter's **identity** lives in its sidecar
  (`chapter.meta.json.chapterID`) and its **order** is its folder's sort position. `chapterID` is **removed from
  `manuscript.meta.json`**; the index becomes a **rebuildable cache/manifest**, never authoritative.
- **Reorder/insert renames at most one folder** (the moved chapter gets a new order-key between its neighbours);
  no tail renumbering. A rare global **rebalance** is the only multi-rename event.
- **Load-time repair:** if the index is missing/inconsistent with disk, ScriviCore **rebuilds it from the
  `chapter-*` folders** (sort by order-key, read each sidecar) — same spirit as I-0066's history self-heal. This
  is also the mechanism that heals I-0072's existing phantom/duplicate damage.

### 7.2 New ScriviCore capability: FileSystem rename/move (approved)
Add to the FileSystem port (`Services.hpp`) a **crash-safe path rename/move**:
`Result<void> renamePath(const AbsolutePath& from, const AbsolutePath& to)` (and a directory move as needed).
Requirements: atomic where the OS allows (`std::filesystem::rename` is atomic within a filesystem); where not,
leave a **detectable half-state** the load-time repair completes. `LocalFileSystem` implements it; mocks/tests
get a fake. This unblocks A4b's single-folder rename on reorder and the migration's folder renames.

### 7.3 Answers to §6 open questions (as decided / as flagged)
1. **Rename primitive now?** — **Yes, build it now** (part of this Epic; A4b + migration both need it).
2. **Index as cache (B3) vs authority?** — **Cache (B3).** Filesystem is authoritative; index is rebuildable.
3. **Scenes symmetry?** — ✅ **DECIDED: Yes, as a follow-on phase within the Epic.** Chapters first (prove the
   order-key + disk-authority pattern), then apply the identical A4b+B3 treatment to scenes' `NNN-slug`
   (`SceneCreator` has the same latent collision). One Epic, phased.
4. **Human-readable label in folder name?** — ✅ **DECIDED: Order-key ONLY** (`chapter-a0m`). Folder names are
   purely mechanical and sortable; the human-readable name lives solely in the sidecar `title`/`displayLabel`.
   Nothing in the folder name is ever parsed for identity or order beyond the order-key itself.
5. **Cross-platform sequencing?** — ✅ **DECIDED: Core-first, then verify per platform.** Land the new scheme +
   rename/move primitive + migration in ScriviCore (ctest-verified), verify on the Linux app, then verify Apple
   opens/migrates a project. A dedicated **`[ScriviCore]` Epic** with per-platform verification sprints. Keep
   `scrivi.h`'s shape stable where possible (the change is on-disk behavior, not API).

### 7.4 Migration (old → new format)
- **Detect:** a project is "old-format" if any `chapter-*` folder name is not a valid order-key (e.g. legacy
  `chapter-NNN` numeric) **or** `manuscript.meta.json` still carries `chapterID` per entry. Detection runs at
  open.
- **Migrate (lazy, idempotent, repair-on-open):** assign each chapter an order-key from its **current index
  order** (authoritative for the *last time* under the old scheme), rename each folder to `chapter-<orderKey>`
  via the new rename primitive, drop `chapterID` from the index (or rewrite the index as a pure ordered path
  manifest / cache). Must be **safe to interrupt and resume** (crash mid-migration leaves a state the next open
  finishes). Old numeric names must keep working until migrated so a half-migrated or unopened project is never
  bricked.
- **Back-compat window:** ScriviCore reads BOTH schemes during the transition; writes only the new one.

### 7.5 Immediate corollary for the in-flight SP-067 work
The chapter-split path (I-0064/I-0069/I-0070) and drag-reorder (I-0067/I-0068) sit on top of the chapter/scene
folder+index model this Epic rebuilds. ✅ **DECIDED: land the I-0067/I-0068 app-layer drag fix** (Qt MoveAction
auto-remove — independent of the on-disk model), and **pause/roll the chapter-structure Issues
(I-0064/I-0069/I-0070/I-0072) into the new Epic** rather than fixing them twice on a model that's being replaced.
I-0071 (last-scene backfill) also folds in, as it's a scene-structure behavior. SP-067's AC4 (drag) can be
verified once the drag fix lands **on a fresh (non-corrupt) project**; its split/renumber ACs defer to the Epic.

### 7.6 Refinement during P2 implementation (2026-07-16): keep chapterID as a self-healing cache
§7.1 said `chapterID` would be **removed** from `manuscript.meta.json`. During P2 the Human decided to **keep it
as a self-healing cache** rather than drop it from the schema. Rationale:
- Order is already fully disk-authoritative (`ManuscriptOrderResolver` iterates `listChaptersByOrder`, ignoring
  the index array), and the index is **rebuilt from disk on open whenever it disagrees** with the on-disk
  folders (`rebuildIndexIfInconsistent`) — de-duplicating and dropping phantom entries (the I-0072 self-heal).
- So the index `chapterID` can no longer diverge (it self-heals to match the sidecars), which means dropping it
  from the schema is **churn without functional gain** — it would ripple through the serde and every consumer +
  test that constructs `ChapterRef{id, path}`, for a field that is now a harmless, always-consistent cache.
- **Net:** B3's *guarantees* are met (filesystem is the source of truth for identity + order; the index is a
  rebuildable, self-healing cache). The literal schema change (`ChapterRef{path}` only) is **deferred** and can
  be done later if desired; it is not required for correctness or for the I-0072 fix.

Consumers still reading `ref.chapterID` (`ChapterRenamer`, `ProjectValidator`, `ExternalChangeScanner`) are
therefore correct as-is, because they read a now-always-consistent (self-healed) index. Migrating them to
resolve identity directly from disk via `ChapterIndex` is an optional robustness follow-up, not a P2
requirement.

**P2 delivered (2026-07-16, ctest 288/288 macOS + Linux):** `util/OrderKey` (fractional keys), `ChapterIndex`
(disk-authoritative helpers + `rebuildIndexIfInconsistent`), `ChapterCreator` order-key slugs (**I-0072
collision fixed**), `ManuscriptOrderResolver` order-by-folder-sort, `ChapterReorderer` = `keyBetween` +
`renamePath` one folder (paths rewritten), and open-time index self-heal. Schema `chapterID` drop + consumer
disk-migration: **deferred** (see above).

### 7.7 P3 delivered (2026-07-16, ctest 290/290 macOS + Linux): legacy-project migration
Implements §7.4 as `migrateChapterOrderKeys(fs, projectRoot)` in `ChapterIndex`, wired into `ProjectOpener`
(step 2a, before the self-heal of step 2b). How it satisfies each §7.4 clause:

- **Intended order source.** For a legacy project the authoritative reading order is the `manuscript.meta.json`
  `chapters[]` **array** order (under the old scheme, reorder shuffled that array, not the folders). Migration
  walks the array and assigns each chapter a fresh ascending order-key via `keyAfter` from a moving cursor.
- **Rename, don't rewrite bodies.** Each out-of-position chapter's folder is renamed to `chapter-<newKey>` via
  the shared `renameChapterFolder` primitive, which `renamePath`s the directory and rewrites the sidecar `slug`
  + every embedded scene `metadataPath`/`contentPath` — so scene bodies stay resolvable (no data loss).
- **Lazy / idempotent / resumable.** It is a **no-op** when the folder-key sort already reproduces the array
  order (every new-scheme project, and any legacy project whose numeric order already matched reading order), so
  it is cheap to call on every open. Only folders still out of position are renamed, and `renamePath` never
  clobbers, so an interrupted run is completed by the next open.
- **Dual-scheme read + collision safety.** Legacy numeric `chapter-NNN` are themselves valid order-keys that
  keep sorting/working until migrated. Generated keys are **letter-prefixed** (`V, k, s, w, …`), whose ASCII
  bytes sort *after* the digit-prefixed legacy folders (`001, 002, …`) — so a target key can never collide with
  an as-yet-unmigrated numeric folder. Back-compat window holds: both schemes read; only the new one is written.
- **Then self-heal.** After renames, `rebuildIndexIfInconsistent` rewrites the index cache to match the freshly
  ordered disk state.

Tests (`integration/ChapterMigrationTests.cpp`, `[EP-027][migration]`): a legacy project whose array order
disagrees with its numeric folder order migrates to the correct reading order with scene bodies intact and is
idempotent on a second open; a project already in order is a verified no-op.

**Remaining phases:** P4 Linux UI verify (VNC), P5 Apple verify, P6 scenes (same treatment as chapters).
