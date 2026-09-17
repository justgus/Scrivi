# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

**Currently: 1 open** — ⚠️ **[I-0222]** (filed 2026-09-17). ✅ **[I-0221] filed AND FIXED 2026-09-17** — ⚠️ **`Resolved - Not Verified`; it stays here until the user verifies it, then moves to `Issue-active.md` or an archive.**

✅ **I-0191 moved to `Issue-active.md` 2026-09-07** — fixed the same day it was filed; ⚠️ **awaiting user verification.**

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| **I-0222** | `[Apple]` ⚠️ **An unavailable world empties every object card with a meaningless `ScriviError 1`** — ⛔ **the card list never asks whether the error is a world-away error**, though the decoder exists; `ScriviError` has no `LocalizedError` so ~15 sites render every error uselessly; ✅ **plus a ruled prefix cleanup.** ⚠️ **The underlying pending behaviour is CORRECT** — this is the message. | **High** | Not Assigned |
| **I-0221** | `[ScriviCore]` ⛔ **A project on ANY non-APFS volume becomes unopenable, REPEATEDLY**: the scene scan parses macOS AppleDouble `._*` sidecars as scene metadata and hard-fails the load — ⚠️ **and the app RECREATES those sidecars on every write, so cleaning them is not durable.** | **Critical** | Not Assigned — ✅ **fixed 2026-09-17, ⛔ not verified** |

---

## I-0221: Scene scan parses AppleDouble `._*` sidecars, aborting project open on FAT/exFAT/SMB — and the app regenerates them

**Status:** ✅ **Resolved - NOT Verified** (fixed 2026-09-17; ⛔ **only the user may mark it Verified**)
**Platform:** macOS (⚠️ **the defect is in ScriviCore — Linux/Windows will inherit it** for any project
written by a Mac onto shared media)
**Component:** `ScriviCore/src/platform/LocalFileSystem.cpp` (where the fix lives — the scan chokepoint);
`ScriviCore/src/manuscript/SceneIndex.cpp` (where the failure surfaced). See *Files Affected*.
**Severity:** ⛔ **Critical** (raised from High 2026-09-17 on the second live pass: ⚠️ **the app recreates the sidecars itself**, so the failure recurs after any manual clean)
**Sprint:** Not Assigned
**Date Identified:** 2026-09-17

**Description:**

Opening a `.scrivi` project stored on a **FAT32, exFAT, or SMB** volume fails with a JSON parse error and
no project opens. macOS cannot store extended attributes natively on those filesystems, so it writes them
into **AppleDouble sidecar files** named `._<original>` beside every real file. `chapter-A/M-scene.meta.json`
acquires a companion `chapter-A/._M-scene.meta.json`.

`listScenesByOrder` selects scene metadata **by filename suffix alone**. `._M-scene.meta.json` ends in
`.meta.json`, so it is selected; `sceneOrderKeyOf` returns a non-empty key from the `._M` prefix, so it is
not skipped; `parseSceneMeta` is then handed a **binary AppleDouble blob** and fails.

⚠️ **The failure is FATAL, not skipped.** `SceneIndex.cpp:73` and `:75` both
`return Result<...>::failure(...)` — so **one** sidecar anywhere in the manuscript aborts the entire
project load. There is no partial open and no repair path.

**Expected Behavior:**

AppleDouble sidecars are not project content and must be **invisible** to every scan. A project opens
identically from any volume. (A corrupt *real* scene file is a separate matter — but even that arguably
warrants skip-and-report over aborting the whole project.)

**Actual Behavior:**

Project open fails with a modal error, and the app returns to the launcher:

```
[json.exception.parse_error.101] parse error at line 1: attempting to parse an empty input;
check that your input string or stream contains the expected JSON
```

⚠️ **The message names no file.** It reports the JSON library's complaint with no path, no scene, no
chapter — nothing to act on. A user cannot discover the cause from the message. (`parse_error.101` is
nlohmann's text for a **non-JSON leading byte**; AppleDouble files begin with the magic `0x00051607`,
not literally empty.)

**Steps to Reproduce:**

1. Copy any `.scrivi` project to a FAT32/exFAT volume with `ditto` or the Finder (**not** `rsync -X`).
2. Confirm the sidecars exist: `find <project> -name "._*.meta.json" | wc -l`
3. Open that project in Scrivi.
4. ⚠️ Open fails as above.

Reproduced 2026-09-17 with `dumas-prose-timelines.scrivi` on `SCRIVI-OTHE`, a `msdos` (FAT32) USB
volume — **55** sidecars in `manuscript/` on the first pass, **115** across the package.
✅ **The project holds 1,179 scenes** (an earlier note in this Issue said 1,224 — that was the count of
`.md` files including non-scene content, corrected here after measuring through `openProject`).

**Evidence (measured, not inferred):**

| Check | Result |
| ----- | ------ |
| Real `*.meta.json` on the volume | **1,237 — all parse cleanly** |
| `._*` AppleDouble sidecars | **55 — none parse** |
| `diff -rq -x '._*'` project vs. source | **0 differences** (the copy is faithful) |
| `engine.openProject` | **succeeded** — the determinate progress bar is `loadSegmentsOffMain` |

✅ **That the copy is byte-faithful and every real file is valid is what proves the sidecars are the
whole cause.** ⚠️ **The 55 are only `manuscript/`** — `objects/` and `worlds/` carry their own.

**Root Cause Analysis:**

`ScriviCore/src/manuscript/SceneIndex.cpp:58-75`. The loop rejects `chapter.meta.json` by exact name and
everything lacking the `.meta.json` suffix, then **accepts whatever remains**:

```cpp
const std::string name = util::filename(absEntry);
if (name == "chapter.meta.json") { continue; }
if (name.size() <= kMetaSuffix.size() || /* ...suffix mismatch... */) {
    continue;   // not a *.meta.json file (e.g. the paired .md)
}
const std::string key = sceneOrderKeyOf(name);
if (key.empty()) { continue; }   // not an ordered scene file
// → reads + parses; ANY failure returns ::failure and aborts the whole load
```

⚠️ **The scan has no notion of a file that is not project content.** It is a suffix allowlist with no
denylist, so it cannot distinguish *"a scene"* from *"a filesystem artifact that happens to end in
`.meta.json`"*.

⚠️ **This is the same shape as the restated-kind-list class in `CLAUDE.md`, one level down**: the
knowledge *"what counts as a project file"* is **restated at each scan site** rather than derived from one
predicate. ✅ **Three sites restate it** (see *Files Affected*), so a fix applied at one leaves two.

**⚠️ THE APP REGENERATES THE FILES THAT BREAK IT (measured 2026-09-17, second live pass):**

⛔ **This is the finding that sets the severity, and it was NOT in the first draft of this Issue.**
Cleaning the sidecars is **not a durable workaround** — Scrivi recreates them itself.

After `dot_clean` removed all 115 sidecars and both packages verified byte-clean, the user opened the
project and relinked the world. ⚠️ **Five sidecars reappeared, timestamped to the relink (17:56), not to
the copy:**

```
manuscript/chapter-….U/._001-scene.meta.json   ⛔ FATAL to the next open
manuscript/chapter-….U/._001-scene.md
worlds/world_character_…/._binding.json
objects/._index.json
history/._log-000001.jsonl
```

✅ **Confirmed AppleDouble by magic bytes `0x00051607` ("Mac OS X"), 4096 bytes each.**

⚠️ **The cycle is self-perpetuating:** open → write anything → sidecar appears → **next open fails**.
✅ **A single scene edit or world relink is enough.** The project was opening cleanly minutes earlier.

✅ **CAUSE PROVEN 2026-09-17 — it is `com.apple.quarantine`.** ⚠️ **My first hypothesis (sandbox
provenance attributes) was WRONG and is recorded here as superseded, not quietly dropped.**

Measured on the FAT32 volume itself:

| Probe | Result |
| ----- | ------ |
| Plain write, no xattr | ✅ **no sidecar** |
| `xattr -w com.apple.test …` on that same file | ⛔ **`._` sidecar appears instantly** |
| `xattr -l` on a file **Scrivi** wrote | ⛔ **`com.apple.quarantine: 0082;…;Scrivi;`** |
| `xattr -l /Applications/Scrivi.app` | ✅ **not itself quarantined** (`com.apple.macl` only) |

⚠️ **So the app is not quarantined but its OUTPUT is.** Scrivi is sandboxed
(`com.apple.security.app-sandbox` + `files.user-selected.read-write`, verified via `codesign -d
--entitlements`), and macOS stamps `com.apple.quarantine` on files a sandboxed app writes to
user-selected locations. On a volume with no native xattr support that stamp **materialises a new
sidecar file**.

⛔ **THEREFORE THE SIDECARS CANNOT BE PREVENTED, only ignored.** `LSFileQuarantineEnabled` defaults
true, and suppressing quarantine is not appropriate for a sandboxed document-based app. ✅ **This is
what settled the fix shape: read-side only.**

**Impact:**

- ⚠️ **Any project on a USB stick, SD card, or SMB share is unopenable** — plausibly common for writers
  who carry work between machines, and the exact case `Scrivi-Worlds`/`SCRIVI-OTHE` exist to test.
- ⛔ **It is not one-shot but RECURRING**: a project cleaned by hand breaks again on the next edit.
  ⚠️ **A writer working from a USB stick loses the project on every second launch**, with no error that
  explains why and no action that keeps it fixed.
- ⚠️ A project round-tripped through a zip, or restored from many backup tools, acquires the same
  sidecars on **APFS**. The trigger is not the filesystem itself but *any* path that materialises `._`
  files, so an internal-disk project is not immune.
- ⚠️ **The error is undiagnosable from the UI** — no filename is reported.
- ⚠️ **It is fail-closed**: one stray file denies access to an entire manuscript.

**Resolution (implemented 2026-09-17):**

✅ **1. One shared predicate.** `util::isIgnorableFilesystemArtifact(std::string_view filename)` in
`ScriviCore/src/util/PathUtils.{hpp,cpp}` — `._*`, `.DS_Store`, `.localized`, `Thumbs.db`,
`ehthumbs.db`, `desktop.ini` (the Windows-shell names case-insensitively, since FAT/SMB volumes are
routinely case-insensitive), `.Spotlight-V100`, `.fseventsd`, `.TemporaryItems`, `.Trashes`,
`.DocumentRevisions-V100`, plus `.`/`..`/empty. ✅ **Placed beside `validateRootPath`, which already
carries the "do NOT re-implement at a call site" rule this Issue is another instance of.**

✅ **2. Applied at the CHOKEPOINT, not at each scan** — ⚠️ **this is a deliberate change from what the
first draft proposed, and the reason matters.** Auditing for the fix found **~20** `listDirectory`
callers, not the three originally identified — `ObjectStore`, `ObjectIndex`, `WorldStore`, `InboxStore`,
`ChapterIndex` and a dozen sites in `ScriviCore.cpp`. ⛔ **Patching three would have left seventeen.**
✅ **Every core scan funnels through `LocalFileSystem::listDirectory`**, so the filter lives there
(`LocalFileSystem.cpp:247`) and fixes all of them at once.

⚠️ **What that deliberately does NOT do**, recorded so it is not mistaken for an oversight: it does not
touch `removeDirectory` (which recurses independently and must still delete sidecars along with a
package), and it hides nothing from a path a caller names **directly** — only from enumeration.

✅ **3. The three scan sites keep an explicit guard as well** (`SceneIndex.cpp`,
`RepairHandlers.cpp`, `ExternalChangeScanner.cpp`). ⚠️ **Redundant against `LocalFileSystem` BY
DESIGN**: tests and other platforms can inject a different `FileSystem`, and the guard must not depend
on which implementation is wired in.

⛔ **4. `SceneIndex`'s fail-closed parsing was NOT changed — it needs a ruling, not a patch.** Its two
sibling scanners `continue` past an unparseable file; `SceneIndex` alone returns `failure`. With (1) and
(2) in place **no sidecar reaches the parser**, so this Issue's symptom is gone either way. ⚠️ **But
whether a single malformed REAL scene should still deny access to an entire manuscript is a genuine
product question** — ⛔ **left open deliberately; it is the user's call, not mine.**

⛔ **5. The unnamed-file error message was NOT changed** — same reason: no longer reachable by this
defect, and worth doing properly rather than incidentally.

✅ **6. Regression tests.** `tests/unit/PathUtilsTests.cpp` (4 cases: artifacts rejected, real files
**not** rejected, shell metadata, volume bookkeeping) and a new
`tests/integration/AppleDoubleSidecarTests.cpp` (2 cases: a littered manuscript, and the single-sidecar
minimal case). ⚠️ **The integration test writes byte-accurate AppleDouble headers itself** (magic
`0x00051607`, padded to 4096) rather than relying on the host filesystem — ✅ **because the existing
suite structurally could not catch this: every fixture builds under `temp_directory_path()`, which on
macOS is APFS, where these files never appear.**

**Files Affected:**

⚠️ **VERIFIED BY READING EACH SITE — they do NOT all behave the same way, and the difference is the
most useful thing in this Issue.**

| Site | Selects sidecars? | On parse failure | Effect |
| ---- | ----------------- | ---------------- | ------ |
| `manuscript/SceneIndex.cpp:58-75` | ✅ yes | ⛔ **`return ::failure`** | ⚠️ **aborts the whole open — the observed defect** |
| `repair/ExternalChangeScanner.cpp:300-312` | ✅ yes | ✅ `continue` | silently skipped |
| `repair/RepairHandlers.cpp:478-488` | ✅ yes | ✅ `continue` | silently skipped |
| `manuscript/SceneReorderer.cpp:19` | ➖ n/a | ➖ | takes a filename **argument**; inherits only via caller |
| `manuscript/ChapterMerger.cpp:21` | ➖ n/a | ➖ | same |

✅ **Three sites select sidecars; only ONE is fatal.** ⚠️ **So the bug is not "sidecars are selected"
— the repair scanners prove selection alone is survivable. The bug is that `SceneIndex` treats an
unparseable file as a FATAL condition** where every sibling site treats it as *"not a scene, move on."*

⚠️ **The two repair sites are not therefore correct** — they skip by *failing to parse*, which is
accidental robustness: they would also silently skip a genuinely corrupt real scene. ✅ **Both defects
are cured by the same explicit predicate**, which is why the fix belongs in one place.

**Related:**

- ⚠️ **Secondary finding, not filed separately:** `ProjectSession.loadAsync`
  (`Scrivi/App/ProjectSession.swift:160-186`) wraps `engine.openProject` in **no**
  `ScriviDiag.measure`, whereas the deprecated `load()` (`:215`) does. ⚠️ **The live path is
  uninstrumented**, so a failing open emits no `[SCRIVI-TIMING]` line at all and the phase had to be
  deduced from the progress bar. Worth folding into whichever Sprint takes this.
- ⚠️ **Observed alongside, NOT a defect:** FAT32's 32 KB cluster rounding inflates this project from
  **9.8 MB to 82 MB** (thousands of small scene files). Harmless at 465 GB free, but a real property of
  Scrivi's many-small-files layout on FAT media.
- **[I-0216]** — `identity_… (new: true)` appears in these logs; that is the known Apple SecureStore gap,
  **unrelated** to this Issue.

**Verification:**

⛔ **`Resolved - NOT Verified`. Only the user may mark this Verified.**

✅ **What was measured 2026-09-17:**

| Check | Result |
| ----- | ------ |
| `ctest` full suite | ✅ **604/604 pass** (598 before, +6 new) |
| New tests **with** the fix | ✅ **6 cases, 38 assertions, pass** |
| ⚠️ New tests with the fix **disabled** | ⛔ **4 fail**, with the production symptom `REQUIRE( after.ok() )` → false |
| ⚠️ **The REAL project on the REAL FAT32 volume** | ✅ **opens: 1,179 scenes** |
| Same project from its APFS source | ✅ **1,179 scenes — identical** |
| Raw `*.md` count on the volume | ✅ **1,179 — nothing hidden** |

⚠️ **THE DISABLED-FIX RUN IS THE ONE THAT MATTERS.** A regression test that passes both with and
without the fix proves nothing; the filter was temporarily stubbed to `return false`, the suite rebuilt,
and the two integration cases reproduced the original failure exactly. ✅ **Then restored and
re-verified.**

✅ **The live pass was run against the actual artifact that produced the defect** —
`dumas-prose-timelines.scrivi` on `SCRIVI-OTHE` (`msdos`/FAT32), ⚠️ **with the five app-regenerated
sidecars still in place, not cleaned first.** ✅ **Scene counts agree across drive, source and raw file
count**, which is what rules out the opposite failure: a filter that hid real scenes would also have
"passed" a bare open check.

⚠️ **WHAT IS STILL UNPROVEN AND NEEDS THE USER'S OWN PASS:** ⛔ **the probe exercises ScriviCore
directly, NOT the shipping app.** ✅ **Open the project from the drive in Scrivi itself**, edit a scene
(so macOS re-stamps quarantine and regenerates a sidecar), quit, and **open it again** — ⚠️ **that
second open is the one that used to fail**, and it is the only thing that tests the full stack.

---

## I-0222: An unavailable world empties every object card with `ScriviError 1` — three defects, one symptom

**Status:** 🔴 Open
**Platform:** macOS. ⚠️ **Defects 2 and 3 are Apple-layer only.** ✅ **Defect 1's prefix cleanup touches
ScriviCore, so Linux/Windows must adopt the same single spelling when they surface world errors.**
**Component:** `Scrivi/Views/Inspector/ObjectCard.swift:219` (⚠️ **the symptom**);
`Scrivi/Engine/ScriviError.swift` (conformance + prefix); `ScriviCore/src/objects/RelationshipStore.cpp`
(prefix only); ~15 SwiftUI call sites
**Severity:** **High**
**Sprint:** Not Assigned
**Date Identified:** 2026-09-17
**Ruling:** ✅ **Defect 1 ruled by the user 2026-09-17 — `worldUnavailable:` is the single prefix.**

**Description:**

With `the-stairs-of-tintagael.scrivi` open from a removable volume and its world bound on that same
volume, **pulling the drive** emptied every object card — Characters, Locations, Chronicles, Factions,
and all Card Lists — each showing:

```
The operation couldn't be completed. ScriviApp.ScriviError 1
```

⚠️ **THE UNDERLYING BEHAVIOUR IS CORRECT AND SHOULD NOT BE "FIXED".** ✅ **The manuscript stayed fully
usable** — the writer navigated scenes with the volume gone (`setCurrentIndex: 6 → 9 → 12 → 0`, normal
`setSel` timings) and every `stampWritingSurface WROTE` succeeded. ✅ **Reattaching the drive restored
all cards within seconds, with no repair prompt and no data loss.** ⚠️ **The edges were held pending
exactly as Doc 3 §4.6 requires. THIS ISSUE IS ABOUT THE MESSAGE, NOT THE MECHANISM.**

⚠️ **THREE ISSUES ARE RECORDED HERE, BUT THEY ARE NOT EQUAL.** ⛔ **Defect 3 alone produced the
symptom** — the card list never asks whether the error means *"the world is away"*, though a working
decoder for exactly that has existed all along. ✅ **Defect 2 is why the fallback text is useless
everywhere else in the app.** ✅ **Defect 1 is a concept-count cleanup, now ruled.**

---

**⛔ DEFECT 1 — two prefixes exist for one writer-visible condition.**

⚠️ **CORRECTED 2026-09-17 AFTER THE FIRST DRAFT OF THIS ISSUE WAS WRONG.** The first draft claimed the
Swift layer knew only `worldPending:` and could not decode `worldUnavailable:`. ⛔ **That is FALSE.**
✅ **`ScriviError.swift:62-88` already defines `unavailablePrefix`, `isWorldUnavailable` and
`unavailableWorldStatus`**, correctly implemented and documented, with an explicit note distinguishing
them from the pending pair. ⚠️ **The ABI was never broken and nothing was undecodable.**

✅ **WHAT IS ACTUALLY TRUE:** both prefixes are emitted correctly and both decode correctly.

| Emitter | Prefix | Swift decoder | Consumers |
| ------- | ------ | ------------- | --------- |
| `objects/RelationshipStore.cpp:192, :323` (graph writes) | `worldPending:` | ✅ `isWorldPending` | **1** (`ObjectCard.swift:333`) |
| `objects/ObjectStore.cpp:95` (**world-scoped reads — this failure**) | `worldUnavailable:` | ✅ `isWorldUnavailable` | **1** (`ObjectDetailSheet.swift:740`) |
| `assets/AssetStore.cpp:47` | `worldUnavailable:` | ✅ same | — |
| `worlds/WorldStore.cpp:902` | `worldUnavailable:` | ✅ same | — |

⛔ **SO THE SYMPTOM IS NOT A DECODE FAILURE — IT IS AN UNASKED QUESTION.** The card list
(`ObjectCard.swift:219`) receives a fully-formed, fully-decodable `worldUnavailable:unmounted` error and
⚠️ **never calls `isWorldUnavailable` at all**, going straight to `localizedDescription`. ✅ **Defect 3
is therefore the whole of the symptom; this defect is about the CONCEPT COUNT, not the plumbing.**

**✅ USER RULING 2026-09-17: collapse to `worldUnavailable:` — "there will be no ambiguity in this
message."**

⚠️ **Scope of that ruling, stated precisely because it is larger than a rename:** `RelationshipStore`'s
two sites change to `worldUnavailable:`, and the Swift `pendingPrefix`/`isWorldPending`/
`pendingWorldStatus` trio is retired in favour of the `unavailable` trio. ⚠️ **`ObjectCard.swift:333`
(the one working pending message) must move to `isWorldUnavailable` in the same change, or the only
correct message in the app breaks.**

⚠️ **ONE THING THE RULING DOES NOT DECIDE, flagged rather than assumed:** the two prefixes currently
distinguish *"the graph is refusing to modify a frozen edge"* from *"a read could not reach the
package"*. ✅ **The writer-facing remedy is identical (reconnect the world), which is the ruling's
point.** ⚠️ **But if any future surface needs to tell a refused WRITE from an unreachable READ, that
information is gone once the prefixes merge** — ✅ **`ErrorCode` and `message` still differ, so it is
recoverable there if ever needed.**

**⛔ DEFECT 2 — `ScriviError` does not conform to `LocalizedError`.**

`ScriviError` (`ScriviError.swift:6`) is a plain `Error` struct carrying `code`, `message`, `detail` and
`path` — ⚠️ **all populated by the C ABI, all discarded at display.** With no `LocalizedError`
conformance, `error.localizedDescription` falls back to Foundation's generic
`"<Module>.<Type> <code>"` rendering. ✅ **`LocalizedError` appears exactly ONCE in the entire app**
(`Engine/ObjectDetail.swift:74`, an unrelated type).

⚠️ **THIS IS NOT LIMITED TO WORLDS.** ⛔ **~15 call sites** use `error.localizedDescription` on a
`ScriviError`: `WorldsView.swift:204,271,300,345,385`, `ObjectCard.swift:219,340,686`,
`WritingToolCards.swift:45,66,76,102`, `ObjectPickerView.swift:223`, `WorldWarningView.swift:106,145`.
⚠️ **Every error this app has ever shown a writer has been rendered this way.** ✅ **The real message
here would have been `"world 'world_character_01a000fb-…' is unmounted"`.**

**⛔ DEFECT 3 — the card list never asks. ⚠️ THIS IS THE WHOLE SYMPTOM.**

`ObjectCard.swift:333` catches `isWorldPending` on `removeFromScene` and produces the right sentence:
*"This link is held pending — its world is unmounted. Reconnect the world to change it."*

⚠️ **The READ path 114 lines earlier (`ObjectCard.swift:219`) — the one that runs when a card
loads — has no such branch**, just `loadError = error.localizedDescription`.

⛔ **Both decoders exist and BOTH have exactly ONE consumer each:** `isWorldPending` →
`ObjectCard.swift:333` (write), `isWorldUnavailable` → `ObjectDetailSheet.swift:740`. ⚠️ **Neither is
reached from the card list**, which is the single most-seen object surface in the app. ✅ **This is the
`project_capability_without_surface` pattern exactly: the capability shipped, the surface was never
built.**

✅ **`WorldStatus.writerDescription` already exists** (`ScriviError.swift:115`) and is used at four
sites — ⚠️ **so the vocabulary for a good message is present and simply not reached on the read path.**

---

**Steps to Reproduce:**

1. Place a project and its bound `.scrivworld` on the same removable volume; open the project.
2. Confirm object cards populate.
3. **Physically eject/pull the volume.**
4. ⚠️ Every card list empties with `The operation couldn't be completed. ScriviApp.ScriviError 1`.
5. Reattach → ✅ cards restore within seconds, no prompt, no loss.

Observed 2026-09-17 on `SCRIVI-OTHE` (FAT32 USB) with `the-stairs-of-tintagael.scrivi` bound to
`Eskandar.scrivworld` on the same volume.

**Root Cause Analysis:**

`ObjectStore::kindDirFor` (`ObjectStore.cpp:85-96`) resolves a world-scoped kind's directory. When the
volume vanished, `WorldStore::resolve` returned `unmounted` and it produced:

```cpp
{.code   = ErrorCode::invalidArgument,                        // == 1
 .message= "world '" + worldID + "' is " + worldStatusName(res.status),
 .detail = "worldUnavailable:" + worlds::worldStatusName(res.status)}
```

⚠️ **`invalidArgument` (1) IS CORRECT AND WAS INVESTIGATED AS A SUSPECTED DEFECT — IT IS NOT ONE.**
An earlier reading of this Issue assumed `ioError` (3) was intended, since a vanished volume looks like
I/O. ✅ **All five pending-world sites use `invalidArgument` consistently**: the volume is not failing,
the *request* is unsatisfiable. ⛔ **Recorded explicitly so it is not "fixed" later by someone
retracing the same reasoning.** ✅ **The code is right; the `detail` prefix is the defect.**

**⚠️ WHY THE TEST SUITE DID NOT CATCH THIS:**

`ScriviInteropTests.swift:2596` (*"worldPending detail parses into a typed status"*) **hand-constructs**
its input:

```swift
let offline = ScriviError(code: 1, message: "frozen", detail: "worldPending:offline")
#expect(offline.isWorldPending)
```

⛔ **It tests the parser against a string the test itself wrote** — ✅ **and the parser was never the
problem: it works.** ⚠️ **What no test asserts is that any VIEW actually calls it.** Both decoders have
exactly one consumer each, and neither is the card list; ⛔ **a unit test of a decoder cannot detect an
un-called decoder.**

⚠️ **The test's own comment says the live path is verified "in SP-102 against a real ejected volume."**
⛔ **That live pass is what this Issue is** — and it found what no fixture could: not a broken
mechanism, but a correct mechanism nothing invokes. ✅ **`feedback_live_pass_finds_what_suites_cannot`,
precisely.**

**Proposed Resolution (NOT implemented — needs a ruling on defect 1):**

1. ✅ **RULED 2026-09-17 — collapse to `worldUnavailable:`.** Change `RelationshipStore.cpp:192,323`;
   retire Swift's `pendingPrefix`/`isWorldPending`/`pendingWorldStatus`; ⚠️ **move
   `ObjectCard.swift:333` to `isWorldUnavailable` in the SAME change** or the app's only correct pending
   message breaks. ✅ **The prefix must be ONE named constant per side of the ABI, not a literal at five
   sites** — restatement at five sites is how they diverged in the first place.
2. Conform `ScriviError` to `LocalizedError`, returning `message` (with `path` when present) instead of
   the Foundation fallback. ✅ **Two lines; fixes all ~15 call sites at once.**
3. Give the card **read** path the same pending branch the write path has, using
   `WorldStatus.writerDescription`.
4. ⚠️ **A regression test that takes `detail` from a REAL `scrivi_*` call**, not a constructed literal —
   otherwise this recurs exactly as it did.

**Files Affected:**

- `Scrivi/Engine/ScriviError.swift:37` — `pendingPrefix` (to retire); `:64` — `unavailablePrefix`
  (to keep); `:6` — the missing `LocalizedError` conformance
- `ScriviCore/src/objects/ObjectStore.cpp:95` — ⚠️ **the throw that produced this symptom**
- `ScriviCore/src/assets/AssetStore.cpp:47`, `ScriviCore/src/worlds/WorldStore.cpp:902`
- `ScriviCore/src/objects/RelationshipStore.cpp:192,323` — the other spelling
- `Scrivi/Views/Inspector/ObjectCard.swift:219` (read, unhandled) vs `:333` (write, handled)
- ~15 `localizedDescription` call sites listed under defect 2
- `Scrivi/Tests/ScriviInteropTests.swift:2596` — the fixture that cannot catch it

**Related:**

- ✅ **[I-0221]** — same live pass. `._binding.json` regenerated at 18:46 during this test, confirming
  that defect's mechanism again in the wild.
- ⚠️ **The world binding itself behaved correctly throughout**: the relink persisted
  (`/Volumes/SCRIVI-OTHE/Eskandar.scrivworld/`, 35 cached objects) and survived the pull and reattach.

**Verification:**

⚠️ **NOT VERIFIED — no fix exists.** ⚠️ **Verification must be a LIVE pull of a real volume**: a
fixture cannot produce the `unmounted` status, which is precisely why this survived a green suite.

---

---

*Last Updated: 2026-09-17 — **two Issues filed from ONE live pass** on a FAT32 USB volume.
✅ **[I-0221]** filed AND fixed the same day (`Resolved - Not Verified`): AppleDouble `._*` sidecars
aborted project open, ⚠️ **and the sandbox's `com.apple.quarantine` stamp REGENERATED them on every
write**, so the failure recurred after any manual clean. ✅ **Fixed at the `listDirectory` chokepoint**
after the audit found ~20 scan callers, not the 3 first identified. ⚠️ **[I-0222]** filed, NOT fixed:
pulling the volume emptied every object card with `ScriviError 1` — ⚠️ **three defects** (two `detail`
prefixes for one condition, no `LocalizedError` conformance, pending handling on the write path only),
⛔ **one of which needs a ruling before any fix.** ✅ **The underlying pending behaviour was CORRECT
throughout: the manuscript stayed usable with the volume gone and everything restored on reattach.**
⚠️ **Both Issues were invisible to a green suite** — fixtures build on APFS and hand-construct their
`detail` strings. Prior note follows.*

*Last Updated: 2026-09-07 — **I-0191 opened** on the user's report of an unexplained folder in the
repo root. ⚠️ **Three garbage-named, EMPTY app-support trees** (dated 2026-08-17) were deleted; ⚠️ **the
mechanism that creates them was NOT fixed.** ✅ **Root cause found by reading the code and REPRODUCED**:
`bootstrapAppSupport` validates `appSupportRoot` in no way, `AbsolutePath` is a bare `std::string`, and
the C ABI's `S()` turns NULL into `""`. ✅ **Test-suite audit (user-requested) came back CLEAN** — every
fixture cleans up via RAII and roots at an absolute `temp_directory_path()`, so ⚠️ **the suite is not the
source**; its real gap is that it can only see its OWN temp dir (AC4). ⚠️ **The exact call site that made
these three folders is NOT identified — recorded as an open question, not guessed.** Prior note follows.*

*Last Updated: 2026-08-20 Removed  references to I-0118 which is verified and does not belong here.  

2026-08-17, later same day (*\*I-0017 ✅ Verified and archived; I-0018 partly fixed and
RESCOPED\*\* — both on the user's report while reviewing this backlog. I-0017 had been fixed and confirmed long
ago and was never filed. I-0018's original complaint (no selection shown on load) is **fixed**; the remaining
behaviour — **the manuscript not scrolling to that selection** — is different from what was reported, so the
Issue is retitled and rescoped rather than left implying the whole thing is broken. ⚠️ It is flagged to be
scoped **together with I-0131 and I-0132**, which are the same underlying question — \*what does it mean to
"be at" a scene?\* — across load, click, and quit. **Backlog is now 1.** Prior note follows.)\*

\*2026-08-17 (**I-0121 and I-0122 ✅ Verified and archived** to
`Verified/Issue-verified-0121-0130.md` at the SP-106 close — both full entries removed from this file in the
same step, which is the discipline the 2026-08-16 note below says was missed twice. **Backlog is now 2:**
I-0017 and I-0018, both 🔴 Open and unassigned. Prior note follows.)\*

*2026-08-16, later same day (*\*I-0058 and I-0112 archived to `Verified/`; I-0121's code fix
applied.\*\* Both Issues had been Verified — 2026-07-09 and 2026-08-11 — and left sitting in this backlog.
⚠️ **The consistency audit earlier the same day did not catch them because it never opened this file**: it
audited `Issue-active.md`, `Issue-Documentation.md` and the `Verified/` archives, and only *grepped*
`Issue-backlog.md`. **An audit scoped to the files that usually go stale will miss the file nobody looks at**
— which is exactly where a Verified Issue goes to be forgotten. I-0121's `rebalancedKeys` guard is now applied
and proven RED-then-GREEN under UBSan (516/516); its CI and coverage halves remain open. Prior note follows.)\*

\*2026-08-16 (**I-0121 opened — ScriviCore CI has been red on every commit since 2026-07-30.**
`rebalancedKeys(1)` divides by zero (`OrderKey.cpp:179`): the ternary guards `n == 0` — already unreachable —
while the divisor is `n - 1`. ⚠️ **Invisible locally by hardware, not by luck**: arm64 returns 0 for integer
division by zero, x86-64 raises SIGFPE, so the developer Mac and the `macos-latest` runner are green while
`ubuntu-latest` crashes. Introduced by **1c42838**, confirmed by `git log -S`, matching the user's report to
the commit. **Assigned to EP-031, to be scoped at the next sprint's planning** (user ruling) together with
adding `-fsanitize=undefined` to CI, which may change the test configuration. Prior note follows.)\*

\*2026-08-11 (I-0112 opened, then **root cause corrected**. Filed as a suspected live-appearance-switch
staleness bug; the user disproved that within minutes — Dark Mode had been active for hours and the app was
launched minutes before the defect was seen, so no switch was involved. Confirmed cause is static: body-text
attribute dictionaries omit `.foregroundColor` (`:517`, `:296-298`) and `textColor` is never set, so AppKit
renders body runs as literal `NSColor.black` against an adaptive dark background. Manuscript-only, as the sole
AppKit text surface. Sprintless/unassigned. I-0017/I-0018/I-0058 unchanged.)\*
