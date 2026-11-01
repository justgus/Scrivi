# Scrivi — History Load: Compaction and Compact Records — Design v0.1

**Status:** ✅ Q1–Q4 RULED 2026-11-01 (user: *"Your recommendations for Q1-Q4 are approved."*): each as its Recommendation reads. Extends [`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`]
(`scrivi.history.v1`, "the history design" below); nothing here changes undo/redo behaviour.
**Epic / Sprint:** [EP-052] / [SP-200]. **Issue:** [I-0332].
**Rulings already given (user, 2026-11-01):** do all of (a), (b) and (c); existing logs stay readable forever and are never
rewritten in place; a design doc first, then a Sprint; order (c) → b1, b2 → b3.

## 1. Why

`scrivi_history_open` replays the WHOLE log on every open. Every project open waits for it (on the load worker since
[I-0331]). Measured on dumas-prose-timelines (34 MB, 12,245 lines, ≈4,600 events, 98 sessions), Debug build:

| Step | Per open |
|------|----------|
| As found | 2.05 s |
| [I-0332] part 1: the consistency pass no longer copies the manuscript per node | 1.35 s |
| (a): `util/Json.cpp` compiled `-O2` in every build (not MSVC) | **0.25 s** |

What is left grows with the log, not with the live history: evicted and purged history is replayed and then thrown away,
and the bytes are dominated by records that store whole texts:

| Bytes in dumas's log | MB | Share |
|---|---|---|
| `structuralPayload` (a structural edit's inverse op; the cut text, escaped twice) | 12.9 | 38% |
| `replace` diffs (Replace All: ONE change per scene from first match to last ≈ the scene twice) | 6.9 | 20% |
| barrier `sceneTexts` (full text after the op) | 5.6 | 16% |
| barrier `sceneTextsBefore` (full text before; the payload holds it again) | 3.7 | 11% |
| floors (each scene's first-seen text) | ≈5 | ≈15% |
| typing diffs | 0.07 | 0.2% |

## 2. (c) Compaction — what the history design already specifies

The history design §6.b: *"on segment rotation (~10k records), eviction, or purge, the live tree is serialized to
`log-(N+1).jsonl` via temp+rename and old segments are deleted"*, and §11: the checkpoint makes *"only the tail replay"*.
Neither was built: `activeSegment_` is fixed at `log-000001.jsonl` and `state.json`'s `activeLogSegment` is written, never
read. This section builds §6.b as written.

### 2.1 The compacted segment is a log, not a new format

`log-(N+1).jsonl` holds the live tree in the records replay already reads, so **replay code is unchanged** and an open
reads one segment:

1. `ctl:session` (the last session's ID) — so the "earlier session" warning ([I-0319]) still sees the previous session.
2. One `floor` per scene the root's floor holds.
3. Every live node as an `event` record, parents before children (a pre-order walk from the root), carrying its original
   `eventID`, `timestamp`, `sessionID` and payload.
4. One `ctl:setPrimary` per fork whose primary child is not the last child (replay's default).
5. `ctl:undo` with `nodeID` = the current node, when it is not the last record's node.
6. One `persisted` record per scene hash the log had ([I-0299]).
7. `ctl:compacted` `{fromSegment, liveNodes, droppedRecords}` — forensic, ignored by replay.

Evicted, purged and pruned nodes, undo/redo moves and superseded `setPrimary` records are not written: they are gone.

### 2.2 Order on disk (crash and pulled-drive safety)

Through the project's write queue (EP-052), in order, so appends made after the compaction land in the new segment:

1. Write `log-(N+1).jsonl` via temp + rename (`atomicWriteTextFile`).
2. Write `state.json` with `activeLogSegment = log-(N+1).jsonl` (atomic, as today).
3. Remove `log-N.jsonl` and any older segment.

At open, the store reads `state.json` FIRST and replays the segment it names. If that file is missing or unreadable it
falls back to the highest-numbered `log-*.jsonl` present. A crash between 1 and 2 leaves `state.json` naming the old,
complete segment (the new one is an orphan, removed at the next compaction); between 2 and 3 the old one is an orphan.
A pulled drive holds the queue (SP-195 Q1) and the steps land in order when it returns.

### 2.3 When

- **At open**, after the load, when the segment is worth compacting (Q1). Queued; the open does not wait for it. This is
  what migrates an existing log (ruling 1): its first compaction rewrites nothing in place, it writes a new segment.
- **At close**, under the same test, so the next open is already compact.

## 3. (b) Records that store changes, not texts

New records use the new shapes; replay reads both, forever (ruling 1). A compaction (§2) writes the new shapes, so an old
log is converted the first time it is compacted, never in place.

### b1 — a diff is a list of changes

`diff: {"hunks":[{"offsetUtf8","removed","inserted"}, …]}`, in ascending offset order, offsets in the OLD text. The old
shape `{"offsetUtf8","removed","inserted"}` is read as one hunk. `Diff` becomes a list of hunks; forward and reverse
apply, `diffMatches`, the I-0107 rebase and the tree's `changeOffsetUtf8`/`changeLength` (first hunk start to last hunk
end) follow. `computeDiff` finds the hunks (Q3). Typing stays one hunk, so its records do not change size.

### b2 — barrier texts as changes

`sceneTexts` (after) is stored as a per-scene diff against the history's text at the barrier's parent; `sceneTextsBefore`
is not stored when it equals that text (Q2). Replay expands both to full texts during the load (one pass from the root,
the same walk the consistency check makes), so everything that reads them in memory is unchanged.

### b3 — the structural payload as JSON, not a string

The core already receives `structuralPayload` as an object and turns it into a string (`scrivi_c_api.cpp:3683`), which
the log escapes again. It is written as an object (`setSubDoc`) and read as either. Core only; no platform changes. The
app's own `rangeJSON` string inside the payload stays as it is (the inverse op passes it back to a core endpoint).

## 4. Tests

- Round trip: a history with every record kind (typing, delete, replace, paste, cut, grouped, barrier with texts,
  structural, setPrimary, purge, evict, undo/redo, persisted) → compact → reopen: the same tree, pointers, heads, primaries
  and `getTree` output; undo and redo give the same texts before and after.
- Crash points: the files as left after each of §2.2's steps → open gives the same tree.
- Old shapes: a log written in the v0.1 shapes opens identically before and after (the current fixtures stay as they are).
- Size and time, through `scrivi_*`: dumas's log and the generated 25,000-event history (`scrivi_make_test_history`), the
  open's time and the segment's bytes before and after, recorded in [SP-200].
- Linux: the same core; smokes and the rig open dumas.

## 5. Questions for ruling

| # | Question | Recommendation |
|---|----------|----------------|
| **Q1** | When is a segment worth compacting? | **When it holds more than 2,000 records not in the live tree** (evicted, purged, pruned, undo/redo, superseded), or more than 10,000 records in all (the history design's rotation figure). A cheap count made during replay. |
| **Q2** | `sceneTextsBefore`: drop it when it equals the parent's history text? | **Yes, and store it (as a diff) only when it differs.** History may lag disk by one typing session (§4.d), so they can differ; the app commits before a structural op, so in practice they are equal. |
| **Q3** | How does `computeDiff` find several hunks? | **A Myers diff on characters, bounded:** at most 64 hunks and only when the changed region is over 4 KB; otherwise today's single hunk. Replace All's scenes are long with few changes, which is where Myers is cheap. |
| **Q4** | Keep the previous segment as a fallback? | **No.** The steps in §2.2 never leave a moment with no complete segment, and a second copy doubles the history's disk use. |

## 6. Implementation notes (2026-11-01, SP-200)

- **Q1:** "more than 10,000 in all" is counted since the LAST compaction (`ctl:compacted` carries the count): a live tree of
  20,000 events always holds more, and recompacting it at every open and close would rewrite it for nothing.
- **Q3:** the diff runs on WORDS (a run of letters, digits or non-ASCII bytes, or any single other byte), not characters:
  a Replace All of 300 short words is ~600 word edits but ~1,800 character edits, past the diff's own bound (1,000 edits;
  its trace grows with the square). A word never splits a UTF-8 character.
- ⚠️ **Q3's 4 KB threshold, measured:** 6.86 of dumas's 6.91 MB of Replace All diffs are under 4 KB (2,334 of 2,340
  records), so it splits almost none of them; at 1 KB they go to 4.6 MB. ✅ **Kept at 4 KB** (user, 2026-11-01: *"no 4KB is sufficient. These departures are accepted."*); the Q1 and Q3 departures above are accepted.
- **§2.1:** a root promoted by eviction is written as an event with no parent and named by a new `ctl:root` record.
- **§2.2 step 3:** the old segment is removed at the NEXT open (any segment `state.json` does not name), not by the queue.
- **§3 b2:** a scene with no history text when the op ran keeps its full text; barrier diffs are one change each (no
  word diff at every load).
- The load's expansion and consistency check share one walk that changes the text map going down and restores it coming
  up; the per-fork copies cost 300 ms on a 906-fork history.

*Last Updated: 2026-11-01 (§6 implementation notes; Q3's threshold measured.)*

*Last Updated: 2026-11-01 (Q1–Q4 ruled as recommended.)*

*Last Updated: 2026-11-01 (created; Q1–Q4 open.)*
