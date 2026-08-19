# Scrivi External Change Repair Matrix v0.2

**Project:** Scrivi  
**Document:** External Change Repair Matrix  
**Version:** 0.2  
**Status:** Approved Direction  
**Date:** 2026-05-21  
**Supersedes:** `Scrivi_External_Change_Repair_Matrix_v0_1.md`  
**Revised:** 2026-07-06 (EP-019 / T-0200) — added §6.21, history store conditions.  
**Revised:** 2026-08-11 (EP-019 / T-0217) — §6.21 reconciled with the shipped implementation: head-hash mismatch causes restated for the relaxed §4.d invariant (a mismatch is **not** by itself evidence of external editing), replayed-purge sub-condition added (I-0110), torn-line behaviour corrected to first-unparseable-line.

---

## 1. Purpose

This document defines how Scrivi detects, stages, and repairs changes made to a `.scrivi` project package outside the normal Scrivi editing flow.

External changes may come from:

```text
manual file edits
external text editors
file manager operations
Git checkout
Git merge
cloud sync
backup restore
drag/drop
scripted modifications
partial copy/restore
```

The purpose of this matrix is to prevent silent data loss while allowing Scrivi projects to remain transparent, inspectable, Git-friendly, and externally editable where appropriate.

---

## 2. Approved Context

This matrix reflects the following approved architecture decisions:

```text
Architecture: Scrivi_Architecture_v0_3.md (supersedes v0.2)
Project format: .scrivi package/folder
Manuscript format: visible .md + .meta.json pairs
Metadata: visible, text-based, app-managed
IDs: UUID v7-style opaque IDs
Git: optional Git-backed snapshots in v1
Indexes: app-local and rebuildable
SQLite: not required in v1
External editing: manuscript text and assets supported; metadata hand-editing not officially supported
Repair pattern: Detect → Stage → Review → Commit
Boundary protocol: JSON-over-std::string (all repair issue data travels by value to the platform wrapper)
```

The repair model itself is unchanged from v0.1. The architecture update (v0.3) does not affect the conditions, classifications, or actions in this matrix. The only architectural consequence for this document is that `RepairIssue` lists are serialized into the JSON result envelope by `ScriviCoreAdapter` and decoded by the platform wrapper — they do not cross the boundary as C++ types.

---

## 3. Core Repair Principles

### 3.1 Preserve data first

When in doubt, Scrivi should preserve all available files and ask the user how to proceed.

### 3.2 Do not silently overwrite external changes

If Scrivi has unsaved in-app changes and the file on disk changed externally, Scrivi must preserve both versions or ask the user.

### 3.3 Do not silently delete unknown files

Unknown files may be staged, ignored, or moved to an inbox, but not deleted without confirmation.

### 3.4 IDs beat filenames

File names and slugs are convenience. Stable IDs remain authoritative.

### 3.5 Metadata is visible but app-managed

Users can see metadata. Scrivi should not assume they safely understand how to edit it by hand.

### 3.6 Repair should be reversible where possible

Before destructive repair actions, Scrivi should create a backup or recommend a Git snapshot when Git is enabled.

---

## 4. Detection Levels

### 4.1 Passive detection

Filesystem watching: file changed, file deleted, file added, folder renamed.

### 4.2 Open-project validation

Required files missing, metadata path points to missing content, chapter list points to missing scene metadata.

### 4.3 Git-aware detection

`git status`, rename detection, checkout-changed files, merge conflict markers, uncommitted changes.

### 4.4 Manual refresh

User-triggered: Refresh Project, Rebuild Project Index, Scan for External Changes.

---

## 5. External Change States

```text
clean
safe external edit
new unregistered file
missing content
missing metadata
possible rename
orphan metadata
schema issue
corrupt metadata
Git state change
merge conflict
unsupported project version
unknown issue

world unavailable          ← added 2026-08-19 (T-0390)
world missing              ← added 2026-08-19 (T-0390)
world identity mismatch    ← added 2026-08-19 (T-0390)
world locked               ← added 2026-08-19 (T-0390)
```

⚠️ **`world unavailable` and `world missing` are DIFFERENT STATES and must not be collapsed.**
`missing` is reported **only on positive evidence** that the package is absent; every other failure —
unreadable, permission denied, volume gone — is `unavailable`. **Guessing `missing` invites destructive
writer remedies for a world that is perfectly intact on an unreachable volume.** See **§6a.0**.

⚠️ **No world state is a deletion state.** None of the four authorises pruning, modifying or tombstoning
an edge into the world.

---

## 6. Repair Matrix

---

### 6.1 Normal pair: `.md` and `.meta.json` both exist and are valid

**Classification:** `clean`

**Behavior:** Open normally. Index or re-index if needed. No repair UI required.

**Automatic:** Update app-local index, update derived stats if content changed through Scrivi.

**Do not:** Rewrite metadata unnecessarily.

---

### 6.2 Markdown changed externally, metadata still valid

**Classification:** `safe external edit`

**Behavior (no unsaved in-app edits):** Reload file or notify user. Update derived stats. Mark index dirty. Set `modifiedBy` to external or unknown.

**Behavior (unsaved in-app edits exist):** Stage conflict. Preserve in-app buffer. Preserve on-disk version. Ask user to compare/choose/merge.

**Suggested user actions:** Reload external version, Keep current Scrivi version, Save current version as copy, Compare changes.

**Git behavior:** Show as unsnapshotted change if Git is enabled.

**Do not:** Silently overwrite external edit. Attribute external edit to current Scrivi identity.

---

### 6.3 Markdown exists, metadata missing

**Classification:** `new unregistered file` or `missing metadata`

**Behavior:** Stage as "Unregistered manuscript file found" or "Manuscript file exists but metadata is missing."

**Suggested user actions:** Import as new scene, Attach to existing scene, Regenerate metadata, Move to inbox, Ignore, Delete after confirmation.

**Automatic repair:** Only if confidence is extremely high and the action is reversible (chapter list references missing metadata, file has expected paired name, Git history shows metadata deletion, content hash matches known previous scene).

**Do not:** Create metadata that reuses an existing scene ID unless confirmed. Delete the Markdown file.

---

### 6.4 Metadata exists, Markdown missing

**Classification:** `missing content`

**Behavior:** Stage as "Scene metadata exists, but manuscript file is missing."

**Suggested user actions:** Restore from Git snapshot, Relink to another Markdown file, Create empty manuscript file, Mark scene as missing, Remove scene from project after confirmation, Ignore for now.

**Automatic repair:** No, except possibly marking the scene as missing in app-local repair state.

**Do not:** Delete metadata automatically. Create empty manuscript file without user confirmation. Remove scene from chapter order automatically.

---

### 6.5 Markdown renamed externally

**Classification:** `possible rename`

**Confidence signals:** Same folder, similar file name, Git reports rename, content hash matches, near-identical content, same heading/title, timestamp relationship.

**Behavior:** Stage as "Possible renamed manuscript file detected."

**Suggested user actions:** Relink metadata to new file, Import new file as separate scene, Restore old filename, Ignore.

**Automatic repair:** Only if Git reports a clear rename and no conflicting candidates exist.

**Do not:** Guess among multiple candidates. Change scene ID.

---

### 6.6 Metadata renamed externally

**Classification:** `possible metadata rename`

**Confidence signal:** `sceneID` inside new metadata matches expected `sceneID`.

**Behavior:** Stage as "Possible scene metadata rename detected."

**Suggested user actions:** Relink chapter scene entry to renamed metadata, Restore old metadata filename, Ignore.

**Automatic repair:** Yes, if the `sceneID` matches exactly and only one candidate exists.

**Do not:** Create duplicate scene entries. Change scene ID.

---

### 6.7 Markdown and metadata both renamed together

**Classification:** `possible paired rename`

**Automatic repair:** Possible if `sceneID` matches, content path inside metadata matches new Markdown filename, and only one candidate exists.

**Behavior:** Stage as "Possible scene file pair rename detected."

**Suggested user actions:** Accept rename, Restore previous names, Import as new scene, Ignore.

**Do not:** Treat as a new scene if it clearly preserves `sceneID`.

---

### 6.8 Chapter folder renamed externally

**Classification:** `possible chapter folder rename`

**Automatic repair:** Yes, if `chapterID` matches exactly and only one candidate exists.

**Behavior:** Stage as "Possible chapter folder rename detected."

**Suggested user actions:** Relink manuscript chapter path, Restore old folder name, Ignore.

**Do not:** Reorder manuscript based on folder name. Change chapter ID.

---

### 6.9 Chapter metadata missing

**Classification:** `missing metadata`

**Behavior:** Stage as "Chapter metadata is missing."

**Suggested user actions:** Restore from Git snapshot, Regenerate chapter metadata from folder contents, Move chapter folder to inbox, Ignore for now, Remove chapter from manuscript after confirmation.

**Automatic repair:** No, unless user selects regeneration.

**Do not:** Delete chapter folder. Remove scenes automatically.

---

### 6.10 Manuscript metadata missing

**Classification:** `missing root manuscript metadata`

**Behavior:** Open repair mode. Stage as "Manuscript structure file is missing."

**Suggested user actions:** Restore from Git snapshot, Rebuild manuscript order from chapter folders, Create new manuscript metadata, Open project read-only, Cancel open.

**Automatic repair:** No.

**Do not:** Invent order silently. Overwrite existing chapter metadata.

---

### 6.11 Project manifest missing

**Classification:** `missing project manifest`

**Behavior:** Do not open as a valid project without repair.

**Suggested user actions:** Restore from Git snapshot, Attempt project recovery from package contents, Open containing folder, Cancel.

**Automatic repair:** No.

**Do not:** Create a new `project.json` over existing content without confirmation.

---

### 6.12 New Markdown dropped into manuscript folder

**Classification:** `new unregistered file`

**Behavior:** Stage as "New manuscript file found."

**Suggested user actions:** Import as new scene in current chapter, Import as new chapter, Move to inbox, Ignore, Delete after confirmation.

**Recommended default:** Import as new scene — but only after user confirmation.

---

### 6.13 New asset file added

**Classification:** `new asset`

**Behavior:** Stage as "New asset found."

**Suggested user actions:** Create asset metadata, Leave unmanaged, Move to inbox, Ignore, Delete after confirmation.

**Automatic repair:** Only if project settings explicitly allow automatic asset registration. Default: staged review.

---

### 6.14 Asset file missing, metadata exists

**Classification:** `missing asset content`

**Behavior:** Stage as "Asset metadata exists, but asset file is missing."

**Suggested user actions:** Restore from snapshot, Relink to replacement file, Mark asset missing, Remove asset metadata after confirmation, Ignore.

---

### 6.15 Metadata JSON corrupt or unreadable

**Classification:** `corrupt metadata`

**Behavior:** Stage as "Metadata file cannot be read."

**Suggested user actions:** Restore from Git snapshot, Open file externally, Move corrupt file aside and regenerate where possible, Ignore for now, Cancel open if critical.

**Automatic repair:** No destructive repair without confirmation.

**Do not:** Overwrite corrupt metadata without backup.

---

### 6.16 Unsupported newer schema version

**Classification:** `unsupported project version`

**Behavior (safe):** Open read-only. Warn user. Avoid writing unsupported files.

**Behavior (not safe):** Refuse to open writable. Show version warning.

**Suggested user actions:** Update Scrivi, Open read-only, Cancel.

**Do not:** Downgrade schema silently. Rewrite newer schema files.

---

### 6.17 Merge conflict markers in Markdown

**Classification:** `merge conflict`

**Behavior:** Stage as "Merge conflict detected in manuscript."

**Suggested user actions:** Open conflict resolver, Open as plain text, Restore one version, Resolve externally.

**V1 scope:** Full merge UI is out of scope. Scrivi may offer safe plain-text visibility and warnings.

---

### 6.18 Merge conflict markers in metadata JSON

**Classification:** `merge conflict / corrupt metadata`

**Behavior:** Stage as critical repair item.

**Suggested user actions:** Restore from snapshot, Resolve externally, Open repair guidance, Cancel open if critical.

**Do not:** Attempt to parse conflicted JSON as normal metadata.

---

### 6.19 Git checkout changed currently open scene

**Behavior (no unsaved in-app edits):** Reload scene. Restore cursor if possible. Mark indexes dirty.

**Behavior (unsaved edits exist):** Preserve in-app version. Preserve checked-out version. Stage conflict. Ask user to choose/merge/save copy.

**Do not:** Overwrite either version silently.

---

### 6.20 Unknown file at project root

**Classification:** `unknown issue` or unmanaged file

**Behavior:** Ignore by default unless file appears dangerous or conflicts with required names.

**Optional staged action:** Move to inbox, Ignore, Delete after confirmation.

---

### 6.21 History store corrupt, missing, or out of sync (`history/`) — *added 2026-07-06 (EP-019, T-0200); revised 2026-08-11 (T-0217) against the shipped implementation*

The `history/` directory holds the undo/redo history and copy buffers
(`scrivi.history.v1` / `scrivi.buffers.v1` — see
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §6 and Appendix A). It is
**app-managed derived state, not canonical content**: the manuscript is never reconstructed from it,
and no history repair may modify any manuscript or metadata file.

**Classification:** `history issue`

**Behavior (by sub-condition):**

```text
history/ absent entirely          → initialize fresh history silently (Info). Normal for
                                    older projects and copies made before EP-019.
Torn final line in active log     → truncate the torn line, continue (Info). Worst case
                                    loses the last uncommitted event.
state.json missing or corrupt     → rebuild the checkpoint by replaying the log segments (Info).
Log segment unparseable           → reset history (fresh scrivi.history.v1), warn the user that
                                    undo history was lost (Warning).
Scene head-hash mismatch          → scene file no longer matches the hash recorded at last close.
                                    Append an `externalChange` barrier for that scene and re-seed
                                    its cached text from disk (Info; surfaced in the history UI as
                                    a barrier, not a repair dialog). See the causes note below —
                                    this is NOT exclusively an external edit.
Replayed purge names the current  → the log contains a purge whose subtree includes the current
node                                node. Re-point `current` to the nearest surviving ancestor
                                    and continue (Info). Must never throw: an unhandled
                                    `unknown node` here loses the project's entire undo history
                                    (I-0110, fixed 2026-08-11).
buffers.json missing or corrupt   → reset to empty buffer set, warn (Warning).
```

> **Causes of a head-hash mismatch (revised 2026-08-11, T-0217).** The original wording named "external
> edit, or crash between undo-apply and save". Since the **§4.d relaxation** (T-0396 — see the history design
> doc §4.a.1/§4.d, user-approved 2026-08-10), there is a third and now more likely cause: a **hard crash
> mid-typing-session**. Because an auto-save no longer commits a history event, disk may lead the recorded
> head by up to one open session's typing; a crash in that window leaves exactly this mismatch on the next
> open. **This is designed behaviour, and the barrier is the intended outcome** — undo stops at the last node
> history can honestly describe rather than walking back past text it never recorded. The failure mode is
> *"undo stops early"*, never *"undo corrupts the manuscript"*, and the manuscript is untouched either way.
>
> Consequence for repair UX: **a mismatch is not by itself evidence of external editing**, and the notice
> must not assert that another app changed the file. It is an Info-level barrier, not a repair dialog.
>
> ⚠️ **The mismatch check is only as good as the hash that was persisted.** Writing the *replayed head*
> hash instead of the *on-disk bytes* made this condition fire on essentially every relaunch (I-0104); a
> stale floor hash in `state.json` then made the same scenes re-flag on every open in a permanent
> barrier→repair→re-persist loop (fixed 2026-08-10, `HistoryStore.cpp:396-436`). Both were false positives
> of this row. When touching the checkpoint, preserve the ordering rule documented there: a hash recorded by
> the save path wins; otherwise hash the floor text, never the replayed head.

**Do not:** modify, restore, or delete any manuscript/metadata/object file as part of history
repair; block writing (history conditions are never Blocking); silently keep history that no
longer matches the scene files; assert that an external application edited a scene on the strength
of a head-hash mismatch alone (see the causes note above).

> **Implementation note (2026-08-11, T-0217).** Replay stops at the **first unparseable line**, keeping
> everything before it (`HistoryStore.cpp:94-99`). This subsumes the torn-final-line row — a torn write is
> the common case, but mid-log corruption degrades the same way rather than resetting the whole history.
> "Log segment unparseable → reset" therefore applies to a log whose **first** records are unreadable, not
> to any single bad line.

**Severity:** Info or Warning only.

---

## 6a. World-package conditions

> **Added 2026-08-19 (EP-031 SP-100, T-0390).** ⚠️ **Before this section the matrix contained ZERO
> occurrences of "world" or "scrivworld"** across 578 lines and 21 conditions — worlds landed in **SP-097**,
> three sprints after its last revision. **A repair pass written against the document as it stood could
> have destroyed a writer's entire relationship graph and satisfied every rule in it.**
>
> Every condition below is **asserted against shipped behaviour**, and each names the test that backs it.
> Where the document and the code disagreed, the disagreement is **filed as an Issue, not fixed here**
> (SP-100 ruling **R3** — a verification sprint that writes fixes stops being one).

### ⚠️ 6a.0 The governing principle: ABSENCE IS NEVER DELETION

**A world that cannot be read is not a world that has been deleted.** This is the single most destructive
mistake available in this area, and it is *silent*: a repair pass that reads "world unavailable" as
"objects deleted" prunes every relationship into that world, reports nothing, and may go unnoticed for
weeks. `Scrivi_World_Data_Separation_v0_1.md` §4.6 names it explicitly.

**Three rules follow, and every condition below obeys them:**

1. ⚠️ **NEVER GUESS `missing`.** `missing` is reported **only on positive evidence** that the package is
   absent — a definitive "does not exist" for the package while its container *does* exist
   (`WorldStore.cpp:308-312`). Any other outcome, including a failed read, degrades to `unavailable`.
2. ⚠️ **An unavailable world FREEZES its edges.** They are held **pending** — never pruned, never
   modified, surviving save byte-for-byte, and restored on reattach with no repair pass.
3. ⚠️ **Identity, not name, decides.** A package's `worldID` is its identity; its folder name is a label.

---

### 6a.1 (§6.22) World package missing entirely — volume unmounted, package moved or deleted

**Classification:** `world unavailable` — or `world missing` **only if positively established**

**Behavior:** `WorldStore::resolve` tries the binding's candidates in order: the **relative** path first
(it survives moving a project and its worlds together), then the absolute one (`WorldStore.cpp:253-264`).
If none yields a readable `world.json`:

- Package **positively absent** *and* its parent directory readable → `WorldStatus::missing`
- Anything else — unreadable, permission denied, volume gone → ⚠️ **`WorldStatus::unavailable`**

**Automatic:** Report status. Cache the world's last-known object names so pending rows show **names, not
bare IDs**.

**Suggested user actions:** Reattach the volume · **Locate…** (relink, §6a.2) · continue working — the
manuscript is fully editable with a world away.

**Do not:** ⚠️ **Prune, modify or tombstone any edge into the world.** Do not report `missing` from a
failed read. Do not offer "remove all references to this world" as a repair for a *transient* absence.

**Backed by:** `WorldTests.cpp:218` (*an absent package reports a status, never an error*) ·
`WorldTests.cpp:234` (*an unresolvable reference falls back to 'unavailable', never a guess*) ·
`WorldTests.cpp:277` (⚠️ *a PRESENT but UNREADABLE package is never reported as 'missing'*)

> ⚠️ **`WorldTests.cpp:277` exists because the naive rule shipped and was wrong.** Deciding `missing` from
> a failed read plus a readable *parent* reported a perfectly intact world as gone — the parent
> (`~/Desktop`) is reachable while a sandboxed package inside it is not.

**Severity:** Warning. **Never Blocking** — a project opens and edits normally with a world away.

---

### 6a.2 (§6.23) ⚠️ `worldID` mismatch — a same-named package with a different identity

**Classification:** `world identity mismatch`

**Behavior:** Resolution reads the candidate's `world.json` and compares `worldID`. On mismatch it
**stops and reports `missing`** rather than substituting (`WorldStore.cpp:295-300`). `relink` refuses
outright with `detail == "worldIDMismatch"` (`WorldStore.cpp:330-338`).

**Suggested user actions:** Locate the correct package · bind the different world as a *separate* world.

**Do not:** ⚠️ **NEVER silently accept a same-named package.** Do not merge, and do not offer "use this
one instead" without stating that it is a **different world**.

> ⚠️ **This is the condition where a plausible convenience is catastrophic.** Two writers sharing a world
> commonly have packages of the same name; substituting one for the other attaches a manuscript's cast to
> a stranger's world, and every subsequent edge is written against the wrong identity.

**Backed by:** `WorldTests.cpp:194` (⚠️ *a same-named package with a DIFFERENT worldID is not the world*) ·
`WorldTests.cpp:306` (*relink verifies identity before accepting a new path*)

**Severity:** Warning (resolution) · Blocking for the relink operation only.

---

### 6a.3 (§6.24) `world.json` corrupt, unparseable, or an unsupported version

**Classification:** `corrupt metadata` (world scope)

**Behavior:** `parseWorld` fails and resolution **continues to the next candidate**
(`WorldStore.cpp:291-292`); if none parses, the status is `unavailable` — ⚠️ **not `missing`**, because a
corrupt file is evidence the package *exists*.

**Automatic:** Nothing. ⚠️ **A world package is never auto-repaired** — it may be shared by other
projects and is not this project's to rewrite.

**Suggested user actions:** Restore `world.json` from backup or version control · Locate a good copy ·
work with the world unavailable.

**Do not:** ⚠️ **Do not regenerate `world.json` from the binding's cached data.** The cache holds display
names for pending rows, **not** the world's identity or epoch; regenerating would mint a *new* identity
and orphan every existing edge. Do not delete the package. Do not prune edges.

**Backed by:** the same `unavailable` fallback as 6a.1 (`WorldTests.cpp:234`).
⚠️ **No test exercises a corrupt `world.json` specifically** — see the gap note in **6a.8**.

**Severity:** Warning.

---

### 6a.4 (§6.25) Binding exists, world is permanently unresolvable

**Classification:** `world unavailable` (persistent)

**Behavior:** Identical to 6a.1 — ⚠️ **there is no "gave up" state, deliberately.** Edges stay **pending**
across repeated opens for as long as it takes. `ResolvedEndpoint` answers `pending()` and `dangling()` as
**distinct** states, and every prune path consults it first.

**Suggested user actions:** Reattach or relink · **Remove world reference** — ⚠️ an explicit, user-initiated
act that unbinds the world **and leaves the package untouched**.

**Do not:** ⚠️ **Do not add a timeout, a retry limit, or an "unavailable too long → prune" rule.** A world
on a drive in a drawer for a year is not a deleted world. Do not let *unbinding* delete anything.

**Backed by:** `ObjectCApiTests.cpp:420` (*world present + endpoint missing is DANGLING, and prunes*) ·
`ObjectCApiTests.cpp:436` (⚠️ *world absent is PENDING, and does NOT prune*) ·
`ObjectCApiTests.cpp:518` (*pending edges survive open and save **verbatim***) ·
`ObjectCApiTests.cpp:555` (*reattaching restores pending edges with no repair pass*) ·
`ObjectCApiTests.cpp:493` (⚠️ *the graph is FROZEN toward an unavailable world — **both directions***) ·
`WorldTests.cpp:326` (*removing a reference leaves the world package untouched*)

> ⚠️ **Both branches of the prune decision are tested, and that pairing is the point.** A test proving
> pending edges survive is worth little without its twin proving genuinely dangling edges still prune —
> otherwise "never prune anything" would pass.

**Severity:** Warning.

---

### 6a.5 (§6.26) Stale write lock in the world package

**Classification:** `world locked`

**Behavior:** World writes take `lock→write→unlock` with a heartbeat. A lock whose heartbeat is **older
than 60 s** is presumed dead and **may be broken**, so a crashed writer blocks others for at most a minute
rather than permanently (`WorldStore.cpp:586-588`). ⚠️ **An unparseable or undated lock is treated as
stale, not as eternal** (`WorldStore.cpp:19-41`) — *"returns a large number when either is unparseable, so
an undated lock is treated as stale rather than eternal."*

**Automatic:** Break a lock older than 60 s; break an unparseable lock. Both are safe: the alternative is
a package locked forever by a file nobody can read.

**Suggested user actions:** Wait · retry. Contention **reports rather than hangs**.

**Do not:** Break a **fresh** lock — another writer may hold it legitimately. Do not delete the package to
clear a lock.

**Backed by:** `WorldTests.cpp:356` (*exactly one writer holds the world lock*) ·
`WorldTests.cpp:376` (*a stale lock is broken; a fresh one is not*) ·
`WorldTests.cpp:405` (*an unparseable lock file does not lock a world forever*) ·
`WorldTests.cpp:108` (*`createFileExclusive`: exactly one of two callers wins*)

> ⚠️ **T-0403 exists because Doc 3 §6.5 assumed a primitive that did not exist.** `AtomicWrite` has no
> exclusive-create path and `rename` **overwrites**, so a lock built on it would have let two writers both
> "win". `createFileExclusive` was added for this.

**Severity:** Info (transient) · Warning if a break was required.

---

### 6a.6 (§6.27) World object file present but absent from the world index

**Classification:** `new unregistered file` (world scope)

**Behavior:** The object index rebuilds **from a directory scan** when missing, corrupt or stale — the same
guarantee as the project index, applied to the world package. One unparseable object file costs **only
itself**, not the whole index.

**Automatic:** Rebuild the index from the scan. ⚠️ This is **safe and expected**: the index is a cache, and
the **files on disk are authoritative**.

**Do not:** Delete an object file because it is unindexed. Fail the open because the index is bad.

**Backed by:** `ObjectIndexTests.cpp:277` (*a MISSING index is rebuilt from a directory scan*) ·
`:295` (*a CORRUPT index is rebuilt rather than failing the open*) · `:323` (*a STALE index loses to
disk*) · `:390` (*one unparseable object file does not cost the whole index*) · `:363` (*rebuilding twice
yields identical content*)

**Severity:** Info.

---

### 6a.7 (§6.28) Worldless project — a world-scoped operation with no world bound

**Classification:** not a defect — ⚠️ **an expected state**

**Behavior:** All ten worldbuilding kinds are world-scoped (Doc 1 §3.0); only `source` is project-scoped.
With no world bound, `kindDirFor` refuses with `detail == "worldRequired"`
(`ObjectStore.cpp:52`, `:469`). ⚠️ **A worldless project otherwise operates silently and completely** —
it opens, edits, saves and closes with no world-related prompt, warning or repair row.

**Suggested user actions:** *(only at the point an operation genuinely needs a world)* Create a world ·
bind an existing one.

**Do not:** ⚠️ **Do not warn, prompt or stage a repair row for a project that simply has no world.**

> ⚠️ **This was tried and withdrawn.** Ruling (a) of 2026-08-14 said a worldless project should prompt to
> create a world on first object creation. **In use the user found the opposite behaviour was right**, and
> the task (T-0410) was ⛔️ **removed as OBE 2026-08-15** — it traced to *no design section, no AC and no
> reported defect*. Superseded by: **operate worldless silently until an operation requires a world.**

**Backed by:** `WorldTests.cpp:338` (*a project with no worlds does nothing world-related*) ·
`ObjectCApiTests.cpp:580` (*a project with no worlds pays no world cost and reports no pending*)

**Severity:** None. **This condition stages no repair.**

---

### ⚠️ 6a.8 Gaps found while writing this section

Per ruling **R3**, disagreements between document and code are **filed, not fixed**. Writing §6a surfaced
the following. **None is a defect in shipped behaviour** — each is a coverage or diagnostic gap:

| # | Gap | Disposition |
| - | --- | ----------- |
| 1 | **No test exercises a corrupt/unparseable `world.json`** (6a.3). The `unavailable` fallback is covered generically, but never for *this* cause. ✅ The **behaviour is correct** — `parseWorld` validates the schema tag and rejects an empty `worldID` (`WorldJson.cpp:31,43-46`) — but **nothing proves a corrupt world file is not auto-regenerated or treated as `missing`.** | ⚠️ **Coverage gap → [I-0135](../Issues/Issue-active.md)** |
| 2 | ⚠️ **`formatVersion` is READ but NEVER COMPARED.** `parseWorld` reads it (`WorldJson.cpp:41`) and **no code anywhere compares it against a supported maximum** — `grep "formatVersion >"` returns nothing. A **newer** world package would be parsed as if current. §6.16 handles exactly this for project files; the world path has no equivalent. | ⚠️ **Behaviour gap → [I-0136](../Issues/Issue-active.md)** |
| 3 | `WorldStatus::offline` and `::unmounted` are **produced nowhere in `ScriviCore/src`** — the core emits only `missing`/`unavailable`; the refinement is Apple-layer (`WorldTypes.hpp:67-68`). | ✅ **Known and by design** — Doc 3 §4.4.1 forbids a platform-specific *model*. Delivered as SP-102/T-0389. **Not an Issue.** |

⚠️ **Gap 2 is the more serious of the two, and it is a behaviour gap rather than a coverage gap.** Forward
compatibility is the one thing a shared, sync-carried package format cannot retrofit: **by the time a
newer world file exists in the wild, the old readers that silently mis-parsed it have already shipped.**

---

## 7. Repair UI Severity Levels

### 7.1 Info

Does not block writing. Examples: new asset found, unknown file at root, unindexed external edit.

### 7.2 Warning

May affect project structure, but writing can continue. Examples: unregistered Markdown file, missing asset, possible rename.

### 7.3 Blocking

Project cannot safely continue in normal editing mode until resolved. Examples: `project.json` missing, `manuscript.meta.json` missing, currently open scene has unsaved conflict, critical metadata corrupt, unsupported newer schema requiring read-only.

---

## 8. Repair Action Policy

### 8.1 Safe automatic actions

```text
mark indexes dirty
rebuild app-local cache
reload clean external edit if no unsaved changes exist
recognize exact ID-preserving metadata rename
record repair issue in app-local state
```

### 8.2 Confirmation-required actions

```text
deleting files
removing scenes from chapter order
creating replacement metadata
creating empty missing content file
relinking ambiguous files
overwriting metadata
moving files out of their current location
discarding unsaved edits
```

### 8.3 Backup-required actions

```text
regenerating metadata
rewriting corrupt metadata
repairing project manifest
mass relinking after Git checkout
```

---

## 9. Git Interaction

### 9.1 Git-enabled projects

Repair UI should prefer Git-aware options: Restore file from snapshot, Compare with last snapshot, Show unsnapshotted changes, Show file deleted since last snapshot.

### 9.2 Git not enabled

Repair options must not depend on snapshot history. Scrivi may suggest enabling Git-backed snapshots but must not require it.

### 9.3 Git conflict scope

Full Git conflict resolution is out of scope for v1. Scrivi detects conflicts and avoids worsening them.

---

## 10. Index/Cache Interaction

Scrivi should mark caches dirty when:

```text
Markdown content changes
metadata changes
chapter order changes
manuscript order changes
objects/assets/comments change
Git checkout changes files
repair action modifies project structure
```

Caches are disposable and should never block repair unless needed for UI display.

---

## 11. Success Criteria

1. External manuscript edits are accepted safely.
2. Missing `.md` and missing `.meta.json` files are handled explicitly.
3. Renames are detected when possible without unsafe guessing.
4. Scrivi does not silently overwrite external changes.
5. Scrivi does not silently delete unknown files.
6. Metadata corruption is recoverable where possible.
7. Git checkouts are treated as normal external project changes.
8. Unsaved in-app edits are preserved during disk conflicts.
9. The writer can continue working when noncritical issues exist.
10. Critical issues open repair mode instead of causing data loss.
11. App-local indexes can be rebuilt after repair.
12. The repair model remains compatible with future encryption and signing.
13. `RepairIssue` data travels by value to the platform wrapper via the JSON result envelope.

---

## 12. Open Issues for Later

1. Repair UI design.
2. File comparison/merge UX.
3. Git conflict handling UX.
4. Asset metadata repair behavior after asset schema approval.
5. Comment anchor repair behavior.
6. Object relationship repair behavior.
7. Encryption-aware repair behavior.
8. Signing-aware tamper detection.
9. Cloud sync conflict behavior.
10. Automated repair confidence thresholds.
11. Repair action operations in the facade API (`applyRepair`, `relinkScene`, etc.).
