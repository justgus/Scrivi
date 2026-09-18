# Verified Issues: I-0221 – I-0230

| ID | Issue | Priority | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| **I-0221** | `[ScriviCore]` ⛔ **A project on ANY non-APFS volume became unopenable, REPEATEDLY.** The scene scan parsed macOS AppleDouble `._*` sidecars as scene metadata and hard-failed the load — ⚠️ **and the app RECREATED those sidecars on every write** (`com.apple.quarantine`, stamped because the app is sandboxed), so cleaning them by hand was not durable. ✅ **Fixed at the `listDirectory` chokepoint.** | **Critical** | Not Assigned | 2026-09-18 |
| **I-0222** | `[Apple]`/`[ScriviCore]` ✅ **An unavailable world reported `ScriviError 1` instead of naming the world.** ✅ **`ScriviError` now conforms to `LocalizedError`** (~15 call sites had been showing Foundation's type-name fallback) ✅ **and the two `worldPending:`/`worldUnavailable:` spellings merged to ONE derived constant** (user-ruled). ✅ **The cards emptying was CLOSED AS ACCEPTED behaviour, not a defect.** | **High** | Not Assigned | 2026-09-18 |

---

## How both were found

⚠️ **BOTH CAME FROM ONE LIVE PASS**, on 2026-09-17, against a real FAT32 USB volume
(`SCRIVI-OTHE`) — ⛔ **and NEITHER was reachable by the test suite as it stood.**

✅ **I-0221 was invisible because every fixture builds under `temp_directory_path()`**, which on macOS
is APFS, where the OS never creates AppleDouble sidecars.

✅ **I-0222 was invisible because the test hand-constructed its own input**
(`ScriviError(detail: "worldPending:offline")`) — ⚠️ **a test that writes the string it then parses can
never discover that the core spells it differently.** `feedback_boundary_tests_not_facade`.

⚠️ **The drive-pull test that produced I-0222 had been deferred as "informational."** ✅ **It was not:
it produced a Critical and a High, and it confirmed the pending-world architecture works.**

---

## I-0221 — what it actually was

| Probe on the FAT32 volume | Result |
| ------------------------- | ------ |
| Plain write, no xattr | ✅ no sidecar |
| Setting **any** xattr on that file | ⛔ `._` sidecar appears instantly |
| `xattr -l` on a file **Scrivi** wrote | ⛔ `com.apple.quarantine: 0082;…;Scrivi;` |
| `xattr -l /Applications/Scrivi.app` | ✅ the app is **not** itself quarantined |

⚠️ **The app is not quarantined but its OUTPUT is**: macOS stamps `com.apple.quarantine` on files a
sandboxed app writes to user-selected locations, and on a volume with no native xattr support that
stamp **materialises a new sidecar file**. ⛔ **So the sidecars could not be prevented, only ignored** —
which is what settled the fix shape.

✅ **Fixed at `LocalFileSystem::listDirectory`**, the chokepoint every core scan funnels through.
⚠️ **The audit found ~20 `listDirectory` callers, not the 3 first identified** — ⛔ **patching three
would have left seventeen.**

✅ **Verified against the real artifact:** the actual project on the actual FAT32 volume, with the
app-regenerated sidecars still in place, opened to **1,179 scenes** — matching both its APFS source and
the raw `.md` count, which rules out the opposite failure of a filter that hides real scenes.

⚠️ **Two things were deliberately NOT changed, and remain open questions rather than oversights:**
⛔ **`SceneIndex` still aborts the whole open on any unparseable file** where its two sibling scanners
`continue` — no sidecar reaches the parser now, but whether one malformed REAL scene should deny access
to an entire manuscript is a product ruling, not a patch. ⛔ **The parse error still names no file.**

---

## I-0222 — and the diagnosis that was wrong twice

✅ **The fix:** `ScriviError` conforms to `LocalizedError`; the `worldPending:`/`worldUnavailable:`
split collapsed to `worlds::worldUnavailableDetail()`, ⚠️ **derived by all five emitters instead of
spelled at each** — restatement at five sites is how they diverged.

⛔ **THE FIRST DIAGNOSIS WAS FALSE AND IS KEPT IN THE RECORD.** It claimed the card list receives a
`worldUnavailable:` error and fails to ask about it. ✅ **Measurement against a real unreachable world
showed neither call in the card's load path fails at all:**

| Call | Actual result with the world away |
| ---- | --------------------------------- |
| `scrivi_list_edges_for` | ✅ `ok:true` — every edge, `otherPending:true`, `otherDisplayName:"Myton"` |
| `scrivi_list_objects` | ✅ `ok:true`, `result:null` → decodes to an empty list, no throw |

⚠️ **A second guess — that the entry filter drops pending rows — was ALSO wrong**:
`ObjectCard.swift:213` admits `known || pendingOfThisKind`, so a pending row of the right kind is
admitted even with an empty index.

✅ **THE LESSON, recorded because it recurred twice in one Issue:** ⛔ **every inference drawn from
reading the code was wrong; only running the endpoints settled it.** ⚠️ **The call that actually threw
`ScriviError 1` was never identified** — ✅ **and with `LocalizedError` in place it would now report the
core's real message, so a recurrence is a NEW observation with new evidence, not this Issue.**

✅ **USER RULING 2026-09-17 — the cards emptying is ACCEPTED, not a defect:** *"The empty list is
acceptable when the project is unavailable."* ⚠️ **An away world's objects genuinely are not available,
so an empty list reports that honestly.** ✅ **Showing cached pending rows is a DESIGN CHOICE if ever
wanted — the machinery already exists** (`ObjectCard.swift:213-218` plus the edge listing's
`otherPending`/`otherKind`/`otherDisplayName`), ⚠️ **so it would be a small Task, never a reopened bug.**

---

## What the drive pull proved WORKS

⚠️ **Recorded deliberately: the pending-world architecture behaved correctly throughout**, and that is
the more important half of the result.

- ✅ **The manuscript never became unreachable.** Scenes were navigated with the volume gone
  (`setCurrentIndex: 6 → 9 → 12 → 0`, normal `setSel` timings) and every `stampWritingSurface WROTE`
  succeeded — the segments were already resident.
- ✅ **Full recovery on reattach** — all cards restored within seconds, **no repair prompt, no data
  loss.** Edges were held pending, never pruned (Doc 3 §4.6).
- ✅ **The relink persisted** through pull and reattach.
- ✅ **`WorldVolumeStatus` diagnosed it correctly as "on a disconnected device"** — ⚠️ **and did so with
  `volumeLabel` EMPTY**, because it derives the volume from the path plus the live mount table
  (`WorldVolumeStatus.swift:97`) rather than trusting that field. ⛔ **An earlier reading of the empty
  `volumeLabel` as a defect was wrong; it is a documented, deliberate design.**

---

## Tests

| Suite | Before | After |
| ----- | ------ | ----- |
| `ctest` (core) | 598 | ✅ **604/604** |
| `xcodebuild test` (interop) | 128 | ✅ **129/129** |

⚠️ **The I-0221 tests were proven to FAIL without the fix** — the filter was temporarily stubbed to
`return false`, the suite rebuilt, and both integration cases reproduced the original failure exactly,
then the fix was restored. ⛔ **A regression test that passes with and without the fix proves nothing.**

✅ **I-0222's fixture test was REPLACED, not amended.** `worldUnavailableDetailCrossesTheBoundary` now
takes `detail` from a real `scrivi_*` call (`openObject` against a deleted package — ⚠️ **not
`listObjects`, which measurement showed does not fail**). ✅ **A second test,
`listObjectsOmitsUnreachableWorldWithoutError`, pins the surprising silent-omission behaviour** so any
future change to it is deliberate rather than accidental.
