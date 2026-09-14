# Active Issues

Issues awaiting **user verification**. An Issue leaves this file only when the user verifies it
(→ `Verified/Issue-verified-XXXX-YYYY.md`, batched in decades of ten) or approves its closure
(→ `Closed/`).

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0147** | `[ScriviCore]` ⚠️ **KNOWN LIMITATION (user-ruled 2026-08-21, ACCEPTED — not to be fixed in SP-116).** ⚠️ **For up to `kStaleSeconds` (60 s) after an interrupted world write, the world is ENTIRELY UNWRITABLE and its abandoned `.partial` is unreclaimable.** When a volume vanishes mid-import the writer dies holding the lock, leaving `.lock` on disk with a **fresh** heartbeat. Reattach the drive quickly — the natural thing to do — and the next write is refused `worldLocked`; ⚠️ **T-0433's sweep runs only AFTER a successful acquire**, so the orphan survives until the lock ages out. **Observed on the real rig 2026-08-21**: drive pulled mid-import, reattached within ~60 s, next import refused and a **2.9 GB** `.partial` remained. ✅ **Both halves verified**: staged fresh lock + orphan → `worldLocked`, orphan stays; waited past 60 s → **acquired and swept**. ⚠️ **This is arguably CORRECT, which is why it is accepted:** `kStaleSeconds` exists precisely because the core cannot distinguish *"writer died"* from *"writer is briefly stalled"*, and guessing wrong means two processes writing a shared world at once. It **self-heals** within a minute and loses no data. **The stronger evidence available — the package's own VOLUME was unmounted, which is far better proof of a dead holder than a quiet heartbeat — is not currently used.** ⚠️ **Deferred to the network-worlds design**, which must revisit *"exactly one winner"* regardless; ruling that inside an asset sprint is how a locking model gets set by accident (the lesson of I-0144). ⚠️ **The eventual UI must not present the 60 s wait as an error** — it is a retryable state. | Low | ⚠️ **Deferred — network-worlds design** | 🟡 **Accepted limitation (2026-08-21)** — ⚠️ **found by the LIVE RIG PASS**; ⚠️ **my own earlier staged-orphan test PASSED because it created the orphan WITHOUT a matching fresh lock** — not the state a real crash leaves |
| **I-0180** | `[Apple]` **The object card repeats the SAME relationship label on every row, and reads as a property of the object.** ⚠️ **Found by the user 2026-08-30 while reviewing the Linux mirror** — ⚠️ **it has been in the macOS app since EP-031 and was never noticed.** `ObjectCard.swift:1032-1036` renders `entry.label` beneath each `displayName`. **Two faults:** (1) ⚠️ **it reads WRONG** — a row showing *Myton at 23 / features* implies Myton features something; the stored edge is *Myton **appears in** scene*, and the core projects the inverse for the queried endpoint, so ⚠️ **the label describes what the SCENE does** and belongs nowhere near the object's name; (2) ⚠️ **it is REDUNDANT** — a scene relates to its objects the same way each time, so the identical word repeats down the whole card while distinguishing nothing. ✅ **Fix (user-ruled, implemented on Linux in SP-126 build 8): hoist the label to the CARD TITLE** — *"Characters (appears in)"* — and drop the per-row line. ⚠️ **Collect the labels from the rows rather than assuming one**: of the seeded vocabulary TWO types constrain to a scene (`appears-in` → *features*, `located-at` → *takes place at*), so a Locations card can legitimately hold both and all must be named. ✅ **Verified from the real project that `cites` never touches a scene** — it runs source→object and surfaces on the Sources card, so it is not a scene predicate at all. ⚠️ **EP-034 is CLOSED**, so this needs its own home rather than being smuggled into a `[Linux]` sprint. | Low | ⚠️ **Needs an Apple home** | 🔵 **Open** |
| **I-0176** | `[Linux]` **A project open at Quit does NOT reopen on next launch.** ⚠️ **Found by the USER on the REAL RIG (T-0476), 2026-08-29** — the first time the Linux app has ever run on real hardware. Linux persists a **recents LIST only** (`RecentsStore`: `addOrUpdate`/`remove`, `recents.json`) — ⚠️ **there is no "was open at quit" state and no session concept at all.** ✅ **Apple has `AppEnvironment.restoreOpenProjects()`** — *"restores all project windows that were open at last quit (R4 / T-0195)… skips any project whose bookmark no longer resolves"*. ⚠️ **Linux has no equivalent.** ⚠️ **Note the Apple version carries an I-0150 guard against restoring real projects under a test run** — ⚠️ **any Linux implementation MUST carry the same guard**, or the rig reopens the user's real work under whatever was just compiled. | Low | SP-123 (found) → ⚠️ **needs an Epic** | 🔵 **Open** |
| **I-0177** | `[Linux]` **A maximized window does not reopen maximized.** ⚠️ **Found by the USER on the REAL RIG, 2026-08-29.** ⚠️ **No window geometry is persisted anywhere in `platforms/linux/`** — not size, position, maximized state, nor the splitter sizes (`EditorShell.cpp:124` hardcodes `{240, 580, 200}` on every launch). ⚠️ **So the inspector/navigator/timeline proportions a writer sets are ALSO lost each launch** — the user reported the window, but the panel layout has the same defect and is arguably more annoying. ⚠️ **Session-scoped-only was a DELIBERATE choice for visibility flags** (SP-078/T-0320 — "a member, not persisted to disk"), ⚠️ **but nothing ruled that GEOMETRY should be discarded**; it was simply never built. ✅ **`QSettings` or the existing `recents.json` are both plausible homes.** | Low | SP-123 (found) → ⚠️ **needs an Epic** | 🔵 **Open** |
| **I-0178** | `[Linux]` ⚠️ **Only ONE project can be open at a time.** ⚠️ **Found by the USER on the REAL RIG, 2026-08-29.** ⚠️ **This is NOT a small gap — it is an entire Apple EPIC missing.** ✅ **EP-018 (R1–R5, verified 2026-06-25) delivered exactly this on Apple**: *"multiple distinct projects open at once, one per window"*, via an `OpenProjectRegistry` and a per-window `ProjectSession`. ⚠️ **The Linux app is single-window by construction** — `ScriviWindow` is one `QMainWindow` hosting a `QStackedWidget` that swaps landing ↔ editor, and `EditorShell` owns ONE `bridge_`, ONE `projectPath_`, ONE `sceneDoc_`. ⚠️ **Multi-project is therefore a STRUCTURAL rework of the shell, not a feature toggle** — and ⚠️ **it is the natural parent of I-0176 and I-0177**, since "restore what was open" and "restore geometry" are both per-window concepts that need a window registry to hang from. ⚠️ **Sizing it as one Issue would repeat the EP-035 AC1 error** (nine ACs collapsed into one); ✅ **it wants its own Epic, scoped from EP-018's actual delivery.** | Medium | SP-123 (found) → ⚠️ **needs its OWN Epic** | 🔵 **Open** |
| **I-0181** | `[ScriviCore]` ⚠️ **An UNMOUNTED volume can be resolved as `missing` — the one status reserved for positive proof of absence.** ⚠️ **Found by INSTRUMENTATION (SP-124 / T-0477), 2026-08-31.** **Root cause:** `WorldStore::resolve` (`WorldStore.cpp:330-348`) decides *package absent **AND** parent directory exists → `missing`*. ⚠️ **An unmounted volume satisfies BOTH whenever the mountpoint directory survives** — ✅ **and a mountpoint is just a directory, whose pre-mount contents reappear.** ✅ **MEASURED on macOS with a hand-specified mountpoint: directory survives, old contents return, `st_dev` matches parent, `statvfs` succeeds — IDENTICAL to Linux.** ⚠️ **The `/Volumes` vs `/media` difference is automounter POLICY (`diskarbitrationd` tidies what it created), NOT an OS semantic** — ⚠️ **so Apple is MASKED by convention, not protected.** ⚠️ **SCOPE CORRECTED TWICE, both by user ruling (2026-08-31):** filed `[Linux]`, re-scoped `[Cross]`, ⚠️ **now `[ScriviCore]` — because it is a CORE RESOLUTION defect, and ⚠️ neither platform currently misreports it to a writer.** ⚠️ **On Linux there is NO world surface at all**: `addWorld`/`relinkWorld`/`getWorldStatus`/`getWorldBinding` are bridged with ⚠️ **ZERO callers**, and the sole `listWorlds` consumer (`EditorShell.cpp:1806`) only recovers a display NAME, always saying *"unavailable"* — ✅ **so the false `missing` cannot reach a Linux writer.** ⚠️ **On Apple the surface exists but `/Volumes` cleanup hides the case.** ✅ **The fix direction is the user's:** ⚠️ **directory EXISTENCE is the wrong question** — ✅ **`st_dev` vs the parent is the indicator that a device is actually mounted there.** ⚠️ **Fix belongs in the CORE's resolve, not in a platform refinement.** | **Medium** | ✅ **T-0498** (SP-124, ⏸️ **paused**) — ⚠️ **assigned 2026-09-04; GATED on T-0477's S3** | 🟢 **RESOLVED - Not Verified (2026-09-10, T-0498)** — ✅ **`FileSystem` gained a device-identity primitive** (`deviceID`, POSIX `st_dev`; ⚠️ **`statvfs` RULED OUT — S2 measured it SUCCEEDING on an unmounted path, reporting the ROOT filesystem's block counts**). ✅ **`resolve` now requires *package absent* AND *container on the SAME DEVICE as its parent* before `missing`.** ⚠️ **THE POLARITY IS EASY TO GET BACKWARDS — I DID, AND THE CONTROL TEST CAUGHT IT.** ✅ **A container with its OWN device has SOMETHING MOUNTED ON IT** — ⚠️ **an absent package there proves only that the world is not on THIS volume, not that it is gone** — ✅ **so `missing` is withheld.** ⚠️ **Same device ⇒ ordinary directory ⇒ absence is REAL and `missing` is honest.** ✅ **`ctest` 585/585 macOS; the new test VERIFIED FAILING against the unfixed core** (pre-T-0498 behaviour restored → `missing`, fix restored → `unavailable`), ⚠️ **with a CONTROL proving an ordinary deleted world STILL reports `missing`** — ✅ **T-0498 must not trade one false status for another.** ⚠️ **KNOWN RESIDUAL, NOT FIXED AND NOT FIXABLE BY `st_dev`: a pulled drive whose mountpoint SURVIVES is INDISTINGUISHABLE from an ordinary directory** — ✅ **both read same-as-parent** — ⚠️ **so that case still reports `missing`.** ✅ **It does not arise on the automounted path a real writer uses: T-0477 S3 MEASURED udisks2 REMOVING the mountpoint it created**, ⚠️ **so `/run/media/<user>/<label>` never reaches this branch at all.** ⚠️ **The HAND-MOUNTED `/mnt` case (fstab, server deployments) REMAINS EXPOSED and needs evidence from the BINDING, not the filesystem, to close.** |
| **I-0192** | `[ScriviCore]` ⚠️ **A world on a PHYSICALLY REMOVED volume resolved `available`, and a full object read SUCCEEDED through it, until a scene change forced a re-resolve.** ⚠️ **Found by the USER on the REAL RIG during T-0477's S3 physical yank, 2026-09-07.** ⚠️ **RE-SCOPED 2026-09-07 from `[Linux]` to `[ScriviCore]` after reading the code — ⚠️ MY ORIGINAL DIAGNOSIS WAS WRONG.** ⚠️ **I filed this as an app-layer 'cached status with no invalidation' and as a placeholder dialog reusing a name it already held. ✅ **The code says otherwise, and the truth is WORSE:** `EditorShell::onOpenObjectRequested` (`EditorShell.cpp:~1790`) calls `bridge_->openObject(...)`, checks `lastCallFailed()`, and ⚠️ **parses the displayed name out of the `objectJson` THAT CALL RETURNED** — its own comment reads *"the object is genuinely read here."* ⚠️ **There is no app-side status cache to go stale.** ✅ **The read is guarded end-to-end**: `ObjectStore::open` → `findByID` → `kindDirFor` (`ObjectStore.cpp:~250`), which calls `WorldStore::resolve()` and ⚠️ **refuses unless status is `available`.** ✅ **And `resolve` caches NO verdict** — it sets `available` only after actually reading and parsing `world.json` from the candidate path (`WorldStore.cpp:337-342`). ⚠️ **SO THE DEFECT IS NOT STALENESS ANYWHERE — the filesystem itself answered successfully for a volume that was physically gone**, and every layer above correctly trusted a correct answer. ⚠️ **The likely mechanism is the PAGE CACHE / unreaped dentries**, which ✅ **the 2b container pass ALREADY measured in a stronger form**: a held FD survived `umount -l` + `losetup -D` entirely, reading and writing fine while the PATH broke instantly. ⚠️ **THIS IS I-0181's SIBLING, NOT ITS OPPOSITE**: I-0181 is `resolve` inferring absence it cannot prove; ⚠️ **I-0192 is `resolve` inferring PRESENCE it cannot prove** — ✅ **and `read succeeded` is no more proof of a mounted volume than `directory exists` is proof of a deleted one.** ⚠️ **T-0498's `st_dev` primitive is plausibly the SAME fix for both** — ⚠️ **but that must be MEASURED, not assumed.** ⚠️ **PARTIALLY MEASURED 2026-09-07 (over SSH, drive already out): the STEADY STATE is CORRECT** — `scrivi_get_world_status` returns ✅ **`unavailable`** (not `missing`), with `packagePath` empty and `lastKnownPackagePath` preserved, ⚠️ **stable across repeated calls**, and ✅ **`binding.json` stores no status key at all** — ⚠️ **so there is no persisted verdict to go stale.** ⚠️ **Therefore I-0192 is a TRANSIENT WINDOW, not a stuck verdict** — ⚠️ **severity lowered accordingly.** ⚠️ **The decay-vs-persist question across the yank itself is STILL OPEN and needs the sampled run.** | **Low** | ⚠️ **Unassigned** — ⚠️ **candidate to FOLD INTO T-0498**, which already owns `resolve`'s device-identity question | 🔵 **Open** |
| **I-0191** | `[ScriviCore]` ⚠️ **`bootstrapAppSupport` would silently `mkdir -p` ANY string — including a RELATIVE one and one full of UNPRINTABLE BYTES — creating junk directories in the process's working directory.** ⚠️ **Found by the USER 2026-09-07**, noticing an unexplained folder in the repo root. ⚠️ **Three garbage-named directories** (`\020v\017k\001`, `@v]m\001`, `8vvm\001`, all dated 2026-08-17) each held the full app-support skeleton — `identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/` — ⚠️ **unmistakably OUR writes.** **Root cause: THREE compounding defects, all confirmed by reading the code.** (1) ⚠️ **`AbsolutePath` is a LIE at the type level** — `Types.hpp:17` aliases it to bare `std::string`, and ✅ **`is_absolute` appeared NOWHERE in `ScriviCore/src`** (grep, 0 hits). (2) ⚠️ **`join()` happily yields a RELATIVE path** (`PathUtils.cpp:11-14`), resolved against the CWD — the repo root for a dev build. (3) ⚠️ **The C ABI turned NULL into `""`** — `scrivi_c_api.cpp:189`, `S(p){return p?p:"";}` — so `scrivi_*(…, NULL)` created `./identity`, `./state/projects`, … and ✅ **reported SUCCESS.** ✅ **REPRODUCED, then fixed and proven RED-then-GREEN.** ✅ **Test-suite audit (user-requested) came back CLEAN** — every fixture cleans up via RAII and roots at an absolute `temp_directory_path()`; ⚠️ **the suite was NOT the source**, its real gap was that it could only see its OWN temp dir. ✅ **AC6 ruled SAFE-AS-WRITTEN, not churned**: all **265** `toUtf8().constData()` uses in `platforms/linux/src` are direct call arguments whose temporary lives to the end of the full expression, and ✅ **none is ever bound to a variable or captured** — the ruling and the ⚠️ **never-hoist rule** are recorded at `ScriviBridge.cpp:34`. ⚠️ **The exact 2026-08-17 call site is still NOT identified** — recorded as an open question rather than guessed. | **Medium** | ⚠️ **Unassigned** — no Sprint active | 🟢 **Resolved - Not Verified (2026-09-07)** — ✅ **579/579 ctest, including under ASan+UBSan**; ⚠️ **user verification pending** |
| **I-0194** | `[ScriviCore]` ⚠️ **`lastKnownPackagePath` returns an UNNORMALIZED TRAVERSAL PATH when a world is unreachable, and it is shown to the writer.** ⚠️ **Found by INSTRUMENTATION during T-0478's live pass, 2026-09-08.** ⚠️ **Offline, both `scrivi_list_worlds` and `scrivi_get_world_status` returned:** `/home/justgus/ScriviProjects/the-stairs-of-tintagael.scrivi/../../../../../../mnt/scrivi-net/worlds/Eskandar.scrivworld` — ⚠️ **six `../` segments escaping the project package.** ✅ **The HEALTHY baseline, minutes earlier on the same world, reported it CLEANLY as `/mnt/scrivi-net/worlds/Eskandar.scrivworld`** — ⚠️ **so the difference is the resolution path taken, not the stored binding.** ⚠️ **THIS IS WRITER-FACING.** ✅ **T-0419 (I-0137) deliberately carries `lastKnownPackagePath` REGARDLESS of status so the UI can say "where we looked"**, and ⚠️ **`WorldsDialog::displayPath()` (`WorldsDialog.cpp:37-40`) passes it straight through — `packagePath.isEmpty() ? lastKnown : packagePath` — with NO normalization**, into `ElidingPathLabel`. ⚠️ **Middle-elision may PARTLY mask it, which makes it easy to miss rather than harmless.** ⚠️ **Severity is Medium, not Low, because the string is the writer's ONLY clue to where a missing world lived** — ⚠️ **and a `../../../../../../` path invites the reader to conclude the app has lost track of its own data.** ✅ **Fix direction: normalize at the CORE, where the path is composed** — ⚠️ **not in each platform's display code, which would re-earn the defect per platform** (`project_abi_boundary_duplication`). ⚠️ **`util::PathUtils` is the natural home; ⚠️ NOTE it must NOT resolve symlinks or touch the filesystem — the volume is by definition ABSENT when this path matters.** | **Medium** | **SP-124** — ⚠️ **found by T-0478's live pass** | 🟢 **RESOLVED - Not Verified (2026-09-09, `e978360`)** — ✅ **FIXED AT THE CORE, where the path is composed** (`WorldStore.cpp:257-279`), ⚠️ **not in each platform's display code** (`project_abi_boundary_duplication`). ✅ **ROOT CAUSE: `fs::weakly_canonical` TOUCHES THE FILESYSTEM, so it FAILS exactly when the volume is unreachable** — ⚠️ **which is precisely when this path is shown to a writer** — ✅ **and its raw-concatenation fallback kept every `..` from the stored relative reference.** ✅ **The fallback now applies `lexically_normal()`, which is PURELY TEXTUAL — no stat, no symlink resolution** — ⚠️ **exactly as this Issue required, since the volume is by definition ABSENT when the value matters.** ✅ **`ctest` 583/583 on macOS (2026-09-10).** ⚠️ **NOT provable from the suite, and the test SAYS SO IN ITS OWN COMMENT:** ⚠️ **`weakly_canonical` SUCCEEDS on a merely non-existent path (ec=0), so absence alone NEVER reaches the fallback** — ⚠️ **the rig's six-`../` value came from a REAL I/O ERROR (`EHOSTDOWN`) that cannot be staged in-process, because `weakly_canonical` calls `std::filesystem` DIRECTLY and bypasses the injectable `FileSystem`.** ✅ **What the test DOES pin is the writer-facing invariant: no `..` ever survives into the value, and normalizing never EMPTIES it** (T-0419/I-0137 carries it so a human can still locate the world). ⚠️ **THE FALLBACK BRANCH IS UNPROVEN BY THE SUITE — it needs a rig pass against a dead share.** |
| **I-0197** | `[Apple]` ⚠️ **Swift READS AND WRITES PROJECT-PACKAGE FILES DIRECTLY, bypassing ScriviCore — violating the architecture's central rule.** ⚠️ **Found 2026-09-10 while auditing the four unbuilt Apple surfaces** ([SP-129]). ⚠️ **CLAUDE.md is explicit: *"No backend logic is reimplemented in Swift. Swift is responsible for UI only."*** ✅ **MEASURED: `35` direct-filesystem/JSON hits across `Scrivi/App` + `Scrivi/Views` + `Scrivi/Engine`, in 11 files.** ⚠️ **They are NOT all equal, and reporting `35` as one number would be alarmist** — ✅ **they fall into THREE classes and only the first is unambiguous:** ---- ⚠️ **CLASS A — A CORE ENDPOINT EXISTS AND IS IGNORED. ✅ ONE confirmed site, and it is on a HOT PATH.** ⚠️ **`TimelineViewModel.loadImportedTimelines` (`TimelineStripView.swift:395-420`) reads `objects/imported-timelines/` with `FileManager.contentsOfDirectory`, then `Data(contentsOf:)` + `JSONDecoder` PER FILE** — ⚠️ **while `scrivi_list_imported_timelines` EXISTS, is bound at `ScriviEngine:924`, and is what LINUX calls.** ⚠️ **This sits on the timeline load path measured at `251 s` in [I-0196]**, ✅ **so it is a PERFORMANCE defect as well as an architectural one.** ✅ **Owned by [T-0502].** ---- ⚠️ **CLASS B — NO CORE ENDPOINT EXISTS, so Swift had nowhere else to go.** ⚠️ **`InspectorLayoutStore` READS AND WRITES `inspector-layout.json` INSIDE THE PROJECT PACKAGE** (`:156`, `:173`, `:304` — a `.write(to:atomic)`) — ⚠️ **and the core has ZERO inspector-layout endpoints (grep: 0).** ⚠️ **THIS IS THE MORE SERIOUS CLASS, not the lesser one:** ⚠️ **an app-layer atomic write into a package whose layout, validation and repair the CORE owns** (Doc: Project Package Structure) ⚠️ **means the core cannot validate, repair, or even SEE this file** — ⚠️ **and the External Change Repair Matrix has no row for it.** ⚠️ **It is also Git-visible project state, so it is shared between platforms that cannot read it.** ---- ⚠️ **CLASS C — EXISTENCE CHECKS ON PATHS THE CORE ALREADY RESOLVED.** ⚠️ **`ObjectCard.swift:980,984`, `ObjectImageSection.swift:106,137`, `ExistingAssetPicker.swift:130` call `FileManager.fileExists` on asset paths.** ⚠️ **`scrivi_list_assets` / `scrivi_import_asset` / `scrivi_remove_asset` exist**, ⚠️ **but none answers *"is this resolved path readable RIGHT NOW"* without opening it.** ⚠️ **These may be LEGITIMATE — a UI checking whether to draw a placeholder is not backend logic** — ✅ **but each needs a RULING, not an assumption**, ⚠️ **and on an unreachable volume a `fileExists` call is exactly the blocking-stat pattern [I-0193] cost 102 s.** ---- ✅ **WHY THIS MATTERS BEYOND TIDINESS:** ⚠️ **every bypass is a place the core's guarantees DO NOT APPLY** — no atomic-write discipline, no repair path, no external-change detection, no `soft`-mount timeout, ⚠️ **and no Linux/Windows equivalent.** ⚠️ **[I-0183] is the precedent for what an unvalidated write into a package costs.** ⚠️ **Scope is CONTAINED (11 files) — ✅ which is why it is worth closing NOW, before it spreads.** | **Medium** | ✅ **SP-130** (EP-039) | 🔵 **Open** |
| **I-0195** | `[Linux]` ⚠️ **PROJECT OPEN BLOCKS THE UI FOR THE WHOLE READ, WITH NO PROGRESS INDICATION — and the cost scales with world size and network speed.** ⚠️ **Reported by the USER on the REAL RIG, 2026-09-08**: opening a project from Recent Projects or the Open dialog became ⚠️ **~10× slower** than before. ✅ **NOT a code regression** — ⚠️ **the share had been remounted `cache=none,actimeo=1,closetimeo=1` for drive-loss testing**, and ✅ **world resolve itself measured a healthy `0.05–0.08 s` throughout**, ⚠️ **so T-0478's changes are NOT implicated.** **Measured on the rig, share healthy, `cache=none`:** ⚠️ **`0.47 s` for a recursive listing** of `Eskandar.scrivworld` and ⚠️ **`1.49 s` to read its `57` files.** ⚠️ **THE DEFECT IS NOT THE MOUNT OPTION — it is that the app has NO ASYNC PATH AND NO PROGRESS UI for a read whose cost is UNBOUNDED.** ⚠️ **`cache=strict` MASKS this today; it does not fix it.** ⚠️ **THE COST ONLY GROWS, along three axes the user named:** ⚠️ **(1) a world accumulates objects over a project's life** — Eskandar is 57 files today and is a TEST world; ⚠️ **(2) a project may bind MORE THAN ONE world**, each paying the same cost; ⚠️ **(3) project AND worlds may BOTH sit on slow networked drives.** ✅ **USER RULING (2026-09-08): waiting is ACCEPTABLE; waiting with a FROZEN, SILENT UI is NOT.** ✅ **The user also supplied the design:** ⚠️ **the file COUNT is known early — before the bulk of the reading — for both the project and each bound world**, ✅ **so `files read / files to read` is a genuine determinate percentage**, ⚠️ **not a spinner pretending to be one.** ⚠️ **Distinct from [I-0193], and BOTH must be fixed:** ⚠️ **I-0193 is the UNREACHABLE case (blocks ~102 s, needs a TIMEOUT); ⚠️ this is the REACHABLE-BUT-SLOW case (completes correctly, needs PROGRESS).** ⚠️ **A timeout alone would ABORT a legitimate slow load; a progress bar alone would show a bar that never finishes.** ✅ **Shared prerequisite: get the read OFF the UI thread** — ⚠️ **the same architectural gap the sprint's FINDING 1 named.** ⚠️ **Classification: PERFORMANCE defect, not functional** — ✅ **the data loads CORRECTLY every time.** | **Medium** | ✅ **SP-128** (⚠️ **assigned 2026-09-10**) — ⚠️ **reported during T-0478's live pass under SP-124** | 🟢 **RESOLVED - Not Verified (2026-09-10, T-0499 + T-0500)** — ✅ **Project open is OFF the UI thread** (`AsyncCall`, reusing [I-0193]'s machinery rather than a second mechanism) ✅ **and reports DETERMINATE progress**: a `QProgressBar` + `n of m scenes`, revealed after 400 ms so a fast local open does not flash it. ✅ **THE COUNT IS A COUNT, NOT AN ESTIMATE** — ⚠️ **`openProject` returns `scenes[]` BEFORE the per-scene body reads that cost the time**, ✅ **so `files read / files to read` is exact** (user ruling, SP-128 §2a: filesystem calls are deterministic and the package layout is OURS). ⚠️ **TIMEOUT IS 10 MIN, DELIBERATELY NOT `AsyncCall::kDefaultTimeoutMs` (5 s)** — ✅ **that figure aborts a DEAD share ([I-0193]); applying it here would ABORT the legitimate slow load THIS Issue exists to support.** ✅ **New smoke `open_progress_smoke` VERIFIED FAILING against an estimate-style implementation.** ✅ **Docker build clean; 23/23 smokes.** ⚠️ **NOT VERIFIED: Docker has NO slow mount, so responsiveness under `cache=none` — the actual defect — is unproven until T-0501's rig pass.** |
| **I-0200** | `[Apple]` ⚠️ **THE APP IS UNUSABLE AFTER THE LOAD: a click in the Scene Navigator or the Manuscript Editor beachballs for SECONDS, and scrolling beachballs, on a 1,153-scene project.** ⚠️ **Reported by the USER 2026-09-12, on the build where [SP-131] cut project open from `~300 s` to `1.06 s`:** *"it still takes much less time to load, but it is still unuseable."* ⚠️ **DISTINCT FROM [I-0196] AND NOT FIXED BY IT** — ✅ **the load is now `1.06 s` wall clock and the console proves it** (`loader.loadAll` `0.60 s`, `TimelineViewModel.load` `0.11 s`, `openScene` avg `0.51 ms`); ⚠️ **this is the cost of EVERY INTERACTION AFTERWARDS.** ---- ⚠️ **ROOT CAUSE — READ IN THE CODE, NOT YET MEASURED AT RUNTIME.** ⚠️ **`SceneNavigatorView.flatRows` (`SceneNavigatorView.swift:641`) is a COMPUTED PROPERTY that rebuilds EVERY ROW — ~1,200 `FlatRow` structs across two allocated arrays — on EVERY SwiftUI body evaluation.** ⚠️ **THREE THINGS MAKE IT RUN CONSTANTLY:** ⚠️ **(1) it reads `loader.allScenes` AND `loader.liveTitles`, both `@Observable`;** ⚠️ **(2) `sceneRow` (`:304`) reads `loader.viewportSceneID` PER ROW, so EVERY row depends on the selection — ✅ ONE CLICK INVALIDATES ALL 1,200;** ⚠️ **(3) the `List` carries `.onMove` (`:117`), which forces SwiftUI to MATERIALISE EVERY ROW rather than lazily — and each row also builds a `.contextMenu` and `.swipeActions` closure set.** ⚠️ **So a single navigator click costs ~1,200 struct builds + ~1,200 view-tree builds + a full `List` diff, ON THE MAIN THREAD.** ---- ⚠️ **A SECOND, SEPARATE COST ON THE SAME SYMPTOM: `tvLen=1823706`** — ✅ **ONE `NSTextView` holds the ENTIRE 1.8 MB manuscript**, ⚠️ **which is why SCROLLING beachballs even when the navigator is untouched**, ⚠️ **and why the console logs `_NSDetectedLayoutRecursion`.** ⚠️ **These are TWO defects presenting as one symptom and must not be assumed to share a fix.** ---- ⚠️ **THIRD: `donateSpotlight` runs on EVERY `willResignActive` with `dirty=0`** — ✅ **MEASURED `355 ms` of blocking main-thread `scrivi_extract_searchable_text` per resign**, ⚠️ **re-reading all 1,154 scene bodies to discover nothing changed.** ✅ **This is [EP-039] AC10 and is ALREADY an AC.** ---- ⚠️ **ALSO REPORTED, POSSIBLY UNRELATED: the Navigation Bar does not render when the window is NOT maximised.** ⚠️ **NO ROOT CAUSE YET — ✅ it is a LAYOUT defect and the other three are COST defects, so the user's expectation that all four share one cause is NOT yet supported by evidence.** ⚠️ **It may share the `_NSDetectedLayoutRecursion` cause; that is a HYPOTHESIS.** | ⚠️ **HIGH** | ✅ **SP-133** (EP-039) — ⚠️ **CORRECTED 2026-09-14: the remaining half is NOT TextKit — it is [I-0204] (`TimelineStripView.buildClusters`, 73% of main-thread samples).** ⚠️ **The earlier attribution to "the manuscript surface is O(DOCUMENT) on TextKit 1" was READ, NOT MEASURED, and the `sample` disproves it.** ✅ **[I-0204] IS NOW FIXED AND THE APP IS USABLE (user-confirmed 2026-09-14).** ⚠️ **WHAT SURVIVES IS SMALLER AND SPLIT OUT: [I-0206]** (⚠️ `~59 ms`/keystroke + offset-linear `setSel`) — ✅ **real O(DOCUMENT) cost, but NOT a hang** | 🟡 **PARTIALLY FIXED - Not Verified (2026-09-12).** ✅ **THE LARGEST COST IS FOUND, FIXED AND MEASURED — and it was NOT the navigator.** ⚠️ **MY FIRST ROOT-CAUSE READING WAS WRONG:** ⚠️ **I blamed `SceneNavigatorView.flatRows` and the `updateNSView` guard, and BOTH were real waste but NEITHER was the freeze.** ✅ **THE USER'S OWN LOG NAMED IT:** ⚠️ **`NSTableView.doubleTapGestureRecognizer has been in possible phase for 48.06 seconds`** — ⚠️ **the table was BLOCKED, not slow.** ---- ✅ **ROOT CAUSE: `ManuscriptTextView.recomputeBoundaries` walked the text storage ONE CHARACTER AT A TIME** (`storage.attribute(.attachment, at: pos, …)` with `pos += 1`) — ⚠️ **`1,823,706` attribute lookups PER CALL on this manuscript** — ⚠️ **from NINE call sites including the NAVIGATOR CLICK path, the SCROLL handler (`:540`) and the TYPING path (`:789`).** ⚠️ **So it cost ~40 ms per click, per scroll event AND PER KEYSTROKE.** ✅ **FIX: `enumerateAttribute` jumps between attribute RUNS, so cost scales with SCENE COUNT (~1,153), not CHARACTER COUNT (1.8 M).** ✅ **MEASURED `40.8 ms` → `0.202 ms` (202x); worst case (every scene its own chapter) `1.28 ms`, still 32x.** ✅ **EQUIVALENCE PROVEN BEFORE SPEED — 17 scenarios, byte-identical ranges, including split/create-adjacent cases (every scene a new chapter, all scenes emptied, adjacent dividers with titles on AND off, insert directly after a heading, unattributed insert, empty first/last scene, single/empty scene); ✅ six also match `rebuildStorage`'s OWN authoritative boundaries, which is the contract `segmentIndex` depends on.** ⚠️ **The user was right to demand this check: my FIRST equivalence test built heading-before-divider and agreed BY LUCK — the real order is divider→heading.** ---- ✅ **CONFIRMED BY THE USER 2026-09-12: the 34–48 s gesture-recognizer warnings are GONE and scrolling improved from ~3 lines to ~7–8 lines before stalling.** ⚠️ **STILL NOT USABLE.** ---- ⚠️ **WHAT REMAINS, and it is ARCHITECTURAL:** ⚠️ **the manuscript is ONE `NSTextView` holding all `1,823,706` characters, and it runs on TEXTKIT 1** — ✅ **MEASURED: 2 `layoutManager` accesses, 0 `textLayoutManager`, so touching `layoutManager` forces the TextKit-1 fallback and the WHOLE document is laid out eagerly.** ⚠️ **The scroll handler then asks that layout manager for `glyphIndex(for:)`.** ⚠️ **This is the deferred "chunk the 1.8 MB NSTextView" item and it needs its OWN sprint — it is a change to the writing surface, not a cost fix.** ⚠️ **`_NSDetectedLayoutRecursion` still fires and is unexplained.** |
| **I-0201** | `[Apple]` ⚠️ **A LAUNCH ARGUMENT SILENTLY PREVENTS THE APP FROM STARTING — no window, no console, no crash, NO THREADS.** ⚠️ **Found 2026-09-12 by the USER, after a full bisect cleared every line of code.** ⚠️ **Passing ANY unrecognised argument in the Xcode scheme (Run ▸ Arguments ▸ Arguments Passed On Launch) makes ⌘R produce NOTHING**: ✅ **the Debug Navigator shows CPU idle, memory flat at `23.8 MB`, disk and network `0`, and ⚠️ NO VISIBLE THREADS** — ⚠️ **which is the tell: a running app always has threads, so the process never really starts.** ---- ✅ **ROOT CAUSE: Scrivi is a DOCUMENT-BASED APP.** ⚠️ **`Info.plist` declares `CFBundleDocumentTypes`** (`com.caposoft.scrivi.project` as Owner, `com.caposoft.scrivi.world` as Alternate) — ✅ **so AppKit treats non-option launch arguments as DOCUMENTS TO OPEN**, ⚠️ **resolves the argument as a file path, finds nothing, and abandons the launch BEFORE the app initialises.** ⚠️ **THE APP PARSES NO ARGUMENTS AT ALL** — ✅ **`grep` for `CommandLine.arguments` in `Scrivi/` returns NOTHING** — ⚠️ **so this is NOT the app rejecting the flag; it never runs.** ---- ✅ **ENVIRONMENT VARIABLES ARE UNAFFECTED and are the correct mechanism**: ⚠️ **they never reach the document-launch path.** ✅ **`SCRIVI_DIAG_TIMING` has worked in that same scheme throughout.** ---- ⚠️ **WHY THIS COST AN AFTERNOON:** ⚠️ **the failure is COMPLETELY SILENT and looks exactly like an app-code hang**, ✅ **so a bisect of the working tree cleared docs, ScriviCore AND Linux one by one and found nothing** — ⚠️ **because the culprit was the ONE VARIABLE THE BISECT NEVER ISOLATED: the scheme itself.** ⚠️ **I twice attributed it to my own code and once to a stale `libScriviCore.a`; ✅ ALL THREE WERE WRONG, and the user found it by removing the scheme argument.** ---- ⚠️ **THIS IS NOT ONLY A TEST-RIG PROBLEM.** ⚠️ **It means NO launch argument can be given to this app from Xcode, by anyone, ever** — ✅ **and nothing anywhere says so.** ⚠️ **A future contributor adding a debug flag will lose the same afternoon.** ✅ **FIX DIRECTION: (a) DOCUMENT the constraint where a flag would be added; (b) prefer ENV VARS for every launch-time switch; ⚠️ (c) if an argument is ever genuinely required, it must be consumed before AppKit's document handling, which is a bigger change than it looks.** | ⚠️ **HIGH** | ⚠️ **Unassigned** — ⚠️ **found during [SP-132]** | 🔵 **Open — ✅ ROOT CAUSE ESTABLISHED BY EXPERIMENT** (add the argument ⇒ no window; remove it ⇒ clean launch, reproduced by the user both ways). ⚠️ **NOT yet fixed: the constraint is undocumented in the codebase.** |
| **I-0202** | `[Apple]` ⚠️ **`MainActor.assumeIsolated` IN THE APP DELEGATE ASSERTS AN ISOLATION APPKIT DOES NOT GUARANTEE — a latent launch/URL crash.** ⚠️ **Found 2026-09-12 while investigating [I-0201]** (⚠️ **NOT its cause — the app never got far enough to call this**). ⚠️ **`AppDelegate.application(_:open:)` (`ScriviApp.swift:23`) and `applicationWillTerminate` (`:29`) both wrap their bodies in `MainActor.assumeIsolated`.** ⚠️ **`assumeIsolated` ASSERTS a precondition; it does NOT establish isolation** — ✅ **if the caller is not actually on the main actor it TRAPS, killing the process.** ⚠️ **The comment claims AppKit *"called on the main thread"*, ✅ which is true for the THREAD but is NOT the same as Swift-6 main-ACTOR isolation**, ⚠️ **and it is asserted rather than checked.** ---- ⚠️ **WHY IT MATTERS: `application(_:open:)` is the FINDER DOUBLE-CLICK PATH** — ✅ **the app declares `CFBundleDocumentTypes`, so a `.scrivi` package double-click routes here** — ⚠️ **and it can arrive DURING LAUNCH, before the SwiftUI `App` has finished establishing its actor context.** ⚠️ **A trap there is indistinguishable from [I-0201]'s symptom: no window, no console.** ---- ✅ **FIX: `Task { @MainActor in … }`, which ESTABLISHES isolation instead of asserting it.** ⚠️ **`onOpenURLs` is already `@MainActor` and the work it does is async anyway, so nothing is lost by hopping.** ⚠️ **CAUTION on `applicationWillTerminate`: it must complete BEFORE the process dies** (it freezes the session manifest, R4/T-0195), ⚠️ **so a `Task` there would NOT be awaited and could silently drop the write** — ✅ **that one needs a different treatment and must be ruled, not blindly converted.** | **Medium** | ⚠️ **Unassigned** — ⚠️ **found during [SP-132]** | 🔵 **Open — ⚠️ LATENT, not observed.** ⚠️ **Filed because it was READ, not measured** — ✅ **no crash has been attributed to it**, ⚠️ **and it must not be assumed to be [I-0201]'s cause.** |
| **I-0203** | `[Apple]` ⚠️ **THE SCENE NAVIGATOR STOPS RENDERING — the writer cannot see it.** ⚠️ **Reported by the USER 2026-09-12:** *"the Scene Navigator hid itself and I couldn't see it."* ⚠️ **SAME defect as the earlier report that "the window, when not maximized, does not render the Navigation Bar."** ---- ✅ **TRIGGER FOUND 2026-09-14 — AND IT IS NOT MAIN-THREAD STARVATION.** ⚠️ **THE PREVIOUS HYPOTHESIS IS DISPROVEN, NOT MERELY UNCONFIRMED.** ⚠️ **Both the user and I believed the UI was so unresponsive that AppKit stopped servicing layout.** ✅ **[I-0204] made the main thread RESPONSIVE (the 33 s `buildClusters` stall is gone, `NSClickGestureRecognizer … possible phase` no longer appears at all) — ⚠️ AND THE CHROME STILL DID NOT RENDER.** ✅ **Starvation is therefore ELIMINATED as the cause.** ---- ✅ **THE ACTUAL TRIGGER, USER-OBSERVED AND REPRODUCIBLE: the `"1 world is unavailable"` WARNING BANNER.** ⚠️ **While the banner is present: the Nav Bar is ABSENT, the Scene Inspector's TAB BAR is ABSENT, the TIMELINE is ABSENT, and a stray title panel reading `"The France of Alexandre Dumas"` sits ATOP the manuscript content, with manuscript text visible all the way to the top of the window.** ✅ **DISMISSING THE BANNER (clicking its X) FIXES ALL FOUR AT ONCE:** ✅ **(1) the stray title panel MERGES INTO the Nav Bar; (2) the Nav Bar appears; (3) the Inspector Tab Bar appears; (4) the Timeline appears.** ⚠️ **Four elements restored by ONE dismissal is a LAYOUT/HIERARCHY fault — the banner is DISPLACING the window chrome — not four independent defects.** ✅ **`_NSDetectedLayoutRecursion` still firing is CONSISTENT with this and no longer needs a starvation explanation.** ---- ⚠️ **RESIDUE AFTER DISMISSAL (likely the same fault, tracked here): a TALL TRANSLUCENT RECTANGLE, ~1.5× the height of the Nav Bar, appears BELOW the Nav Bar**, ⚠️ **obscuring manuscript text at the top of the viewport and the top of the Scene Inspector**, ⚠️ **spanning from the LEFT edge of the Manuscript View to the RIGHT edge of the Scene Inspector.** ✅ **PRESENT ONLY WHEN NOT IN FULL SCREEN.** ⚠️ **Its identity is UNKNOWN — it must be identified in the view hierarchy, not guessed.** ---- ⚠️ **MY EARLIER `NavigationSplitView` SIZING HYPOTHESIS REMAINS REJECTED** (`EditorView.swift:201`, `.balanced`, no `navigationSplitViewColumnWidth`, `columnVisibility` wired only to the iOS branch at `:161`). ✅ **Still worth tidying**, ⚠️ **but it does not explain a banner-gated trigger.** ⚠️ **NOT INTRODUCED BY [SP-132]** — ✅ **`SceneNavigatorView.swift` is UNMODIFIED from HEAD** (verified), ⚠️ **and the first report predates that sprint.** ---- ⚠️ **WHY IT MATTERS: the navigator is the ONLY way to move between 1,156 scenes.** ⚠️ **A writer who loses it loses navigation entirely, with no affordance to bring it back — ✅ and the ONE affordance that restores it (dismissing an unrelated warning) is undiscoverable.** ---- ✅ **CODE READ 2026-09-14 (first time — the record previously carried NO code reading).** ✅ **FOUR STRUCTURAL FACTS, ESTABLISHED:** ✅ **(1) THE BANNER IS A SIBLING OF THE MANUSCRIPT INSIDE A `VStack`** (`EditorView.swift:283-302`): `ManuscriptTextView` carries `.frame(maxWidth: .infinity, maxHeight: .infinity)` and the banner is appended BELOW it in the same `VStack`. ⚠️ **I HYPOTHESISED a SwiftUI over-commit here (greedy sibling squeezes the chrome) — ✅ **THE VIEW HIERARCHY DISPROVED IT** (see CAPTURE below): the chrome is STRUCTURALLY INTACT in both states.** ✅ **(2) `manuscriptDetail` — the view the banner lives inside — IS ALSO THE VIEW CARRYING `.navigationTitle` / `.navigationSubtitle`** (`:229-230`). ⚠️ **That is EXACTLY the pairing the user observed: the stray *"The France of Alexandre Dumas"* panel is the NAVIGATION TITLE rendering as a separate band, and dismissing the banner lets it MERGE BACK into the window title area.** ✅ **(3) THE macOS EDITOR HAS NO TOOLBAR AT ALL.** ⚠️ **`.toolbar { editorMenuToolbar }` is attached ONLY on the iOS branch (`:189`), and `editorMenuToolbar` is itself inside `#if os(iOS)` (`:483-504`).** ⚠️ **This is CORRECT and deliberate — macOS uses the real menu bar — ✅ but it means the thing the user calls the *"Nav Bar"* is the WINDOW TITLE AREA, not a toolbar, so any fix aimed at a toolbar would be aimed at nothing.** ✅ **(4) THE WINDOW IS A BARE `NSHostingView` IN A `.titled` WINDOW** (`ProjectWindowManager.swift:89-93`) — ⚠️ **no `NSToolbar`, no `titlebarAppearsTransparent`, no split-view controller.** ✅ **So SwiftUI's title/subtitle have no AppKit toolbar to render into, which is why they surface as an in-content band.** ---- ⚠️ **THIS IS A HYPOTHESIS WITH EVIDENCE, NOT A PROVEN CAUSE.** ⚠️ **It explains all five observations (missing title area, missing inspector tabs, missing timeline, the stray title panel, and all of them returning together on dismissal) — ✅ but it has NOT been confirmed against the running view hierarchy.** ⚠️ **THREE CODE-READ DIAGNOSES WERE WRONG EARLIER IN THIS SAME INVESTIGATION** ([I-0204]'s cause was found by `sample`, not by reading) — ⚠️ **so this must be CONFIRMED by inspecting the live hierarchy (Xcode's Debug View Hierarchy with the banner up) BEFORE any fix is written.** ⚠️ **The translucent rectangle remains UNIDENTIFIED and is the specific thing that inspection must name.** ---- ✅ **VIEW HIERARCHY CAPTURED 2026-09-14 (user-supplied, both states) — THIS SUPERSEDES THE CODE-READ HYPOTHESIS ABOVE.** ⚠️ **TWO OF MY READINGS WERE WRONG AND ARE RECORDED AS SUCH:** ⚠️ **(i) the `VStack` over-commit — ✅ DISPROVEN: `NSToolbarView`, `NSTitlebarView` and the widgets are PRESENT AND IDENTICAL in both captures; nothing is squeezed out of the layout;** ⚠️ **(ii) `TUINSWindow` as the translucent rectangle — ✅ DISPROVEN: it is present in BOTH states (it was merely omitted from the first dismissed capture).** ---- ✅ **THE ONLY REAL STRUCTURAL DIFFERENCE, inside `NSTitlebarContainerView`:** ✅ **BANNER SHOWN → TWO `NSScrollViewMirrorView` + `NSPortalView` + `_NSPortalLayerBackedView` triples.** ✅ **BANNER DISMISSED → ONE triple, ⚠️ PLUS an `NSScrollPocket`.** ---- ✅ **THE TRANSLUCENT RECTANGLE IS `NSScrollPocket`** (user-identified), ⚠️ **and it does NOT live in the titlebar branch at all:** `NSSplitViewController → NSView → NSSplitView → `_NSSplitViewItemViewWrapper` → **`NSTitlebarBackgroundView`** → **`NSScrollPocket`**. ✅ **That placement — INSIDE the split-view item — is exactly why it spans from the manuscript's left edge to the Scene Inspector's right edge and obscures the top of BOTH**, ⚠️ **and why it is ABSENT IN FULL SCREEN (no titlebar to inset under).** ---- ✅ **MECHANISM (one cause, both symptoms): AppKit's TITLEBAR SCROLL-EDGE INTEGRATION is binding to the WRONG scroll view, and RE-BINDS when the banner changes the subview set.** ⚠️ **A mirror triple is a live portal of a scroll view painted BEHIND the titlebar; TWO triples means AppKit is mirroring TWO scroll views into ONE titlebar** — ✅ **which is the title area rendering as a floating band over content rather than as chrome.** ✅ **`NSScrollPocket` is the companion inset AppKit carves so scrolled content can slide under the titlebar.** ⚠️ **So this is NOT a SwiftUI sizing fault and NOT the banner's own layout — it is automatic AppKit chrome/scroll integration resolving an AMBIGUOUS target.** ---- ⚠️ **WHAT IS STILL NOT ESTABLISHED: WHICH scroll views are being mirrored, and WHY the set is ambiguous.** ⚠️ **The editor contains at least the manuscript `NSScrollView` (`ManuscriptTextView.swift:126`) and the navigator's `List`; ✅ the next step is to identify the two mirrored targets by ADDRESS in a capture, not by inference.** ⚠️ **DO NOT 'FIX' THIS BY RESTYLING THE BANNER — the banner is only the trigger that changes the subview set.**  ---- ✅ **CONFORMANCE STUDY WRITTEN 2026-09-14** → [`../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md`](../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md) (⚠️ **DRAFT, awaiting ruling**). ✅ **It SOURCES the mechanism from Apple:** ✅ **the scroll edge effect is applied AUTOMATICALLY beneath toolbar items, titlebar accessories and split item accessories** ([WWDC25 §310](https://developer.apple.com/videos/play/wwdc2025/310/)), ⚠️ **and macOS 27 resolves `automatic` to the HARD style WHEN FREE-FLOATING TITLE TEXT IS PRESENT** ([WWDC26 §289](https://developer.apple.com/videos/play/wwdc2026/289/)) — ✅ **which is Scrivi's exact case (`.navigationTitle` with NO `NSToolbar` anywhere in the app).** ✅ **F3: every bar here is a plain `VStack` SIBLING — banner, Inspector tab bar AND Timeline all share one `VStack` — so none of them insets the safe area or extends the edge effect**, ⚠️ **which is why ONE dismissal moves all four.** ⚠️ **THE STUDY DOES NOT UPGRADE THIS RECORD:** ⛔ **the two mirrored scroll views are now identified STRUCTURALLY (the navigator `List` at `SceneNavigatorView.swift:104` and the manuscript `NSScrollView` at `ManuscriptTextView.swift:126`) but STILL NOT BY ADDRESS** — ⚠️ **that capture remains REQUIRED before any fix.** | **Medium** | ⚠️ **Unassigned** — ✅ **NO LONGER BLOCKED on [I-0200]** (⚠️ **that gate assumed starvation, which is now disproven**) | 🔵 **Open — ✅ TRIGGER REPRODUCIBLE, ROOT CAUSE NOT YET READ IN THE CODE.** ⚠️ **The banner's placement in the view hierarchy is the thing to examine; ⚠️ do NOT fix from this description without confirming it in the hierarchy.** |
| **I-0205** | `[Apple]` ⚠️ **THE `"1 world is unavailable"` WARNING BANNER IS SHOWN FOR A CONDITION THAT IS NORMAL, AND IT DISPLACES THE WINDOW CHROME WHILE SHOWN.** ⚠️ **Observed by the USER 2026-09-14 on the `dumas-prose` fixture.** ---- ⚠️ **TWO SEPARABLE COMPLAINTS, FILED TOGETHER BECAUSE ONE BANNER CAUSES BOTH:** ✅ **(1) THE LAYOUT DAMAGE is tracked in [I-0203]** (Nav Bar, Inspector tab bar and Timeline all absent while the banner is up; all four restored by dismissing it) — ⚠️ **not restated here.** ⚠️ **(2) THE BANNER'S EXISTENCE IS ITSELF QUESTIONABLE, and that is THIS record.** ---- ⚠️ **A world being unavailable is NOT NECESSARILY AN ERROR STATE.** ✅ **Per `project_unlinked_world_objects_are_normal`: a shared world's objects are unlinked in every new project BY DEFINITION, and an absent world is the EXPECTED condition when the volume holding it is simply not mounted.** ⚠️ **Presenting that as a persistent warning that must be manually dismissed — ✅ and which BREAKS THE WINDOW CHROME until it is — is disproportionate to the condition.** ---- ⚠️ **WHAT MUST BE ESTABLISHED BEFORE ANY FIX (do NOT skip to the fix):** ⚠️ **(a) WHICH world is unavailable and WHY** — ✅ **the fixture is on the Desktop, not on the removable rig drive, so a missing USB world is NOT the obvious explanation and must not be assumed;** ⚠️ **(b) whether the banner is CORRECT-BUT-UGLY (a real missing world, badly presented) or FALSE (a world that is present being reported absent).** ⚠️ **These lead to OPPOSITE fixes and the evidence does not yet distinguish them.** ---- ⚠️ **WHY IT MATTERS: ✅ if the banner is correct, a writer is being punished with a broken window for a normal condition; ⚠️ if it is false, a world-availability check is wrong and that is a DATA-facing defect, not a cosmetic one.**  ---- ✅ **CONFORMANCE STUDY 2026-09-14** → [`../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md`](../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md) §6. ⚠️ **THE STUDY EXPLICITLY DOES NOT ANSWER THIS RECORD** — ✅ **it explains the banner's LAYOUT DAMAGE (that is [I-0203]) and says NOTHING about whether the banner should exist.** ⚠️ **Questions (a) and (b) above STAND and must be answered from evidence before the banner is touched.** | **Medium** | ⚠️ **Unassigned** | 🔵 **Open — ⚠️ NOT DIAGNOSED.** ⚠️ **Reported from observation only; ✅ no code has been read for this record and none should be cited until it has.** |
| **I-0206** | `[Apple]` ⚠️ **EVERY KEYSTROKE COSTS ~59 ms INSIDE APPKIT, AND `setSelectedRange` IS LINEAR IN DOCUMENT OFFSET — the manuscript surface is still O(DOCUMENT) on TextKit 2.** ✅ **MEASURED 2026-09-14, on the build that fixed [I-0204].** ⚠️ **These were MASKED by [I-0204]'s 33 s stall and became visible only once it was removed.** ---- ✅ **TYPING — `[SCRIVI-EDIT] keyDown(…)` is `57.8–88.6 ms`, REMARKABLY CONSTANT across ~70 logged keystrokes.** ⚠️ **OUR OWN WORK IS NOT THE COST:** ✅ **`[SCRIVI-KEY] total=0.5–1.3 ms` and `didChangeText=0.6–1.4 ms`** — ⚠️ **so ~58 ms of every keystroke is AppKit, OUTSIDE our instrumentation.** ⚠️ **This caps sustained typing at ~17 keys/sec** ⚠️ **and is the most likely thing a writer FEELS as sluggishness, because it is paid on EVERY character.** ---- ✅ **NAVIGATION — `[SCRIVI-NAV] setSel` SCALES WITH DOCUMENT OFFSET** (it did NOT vary before, because the 33 s stall dominated): ✅ **idx 79 / offset 123,465 → `5.4 ms`; idx 327 / offset 513,559 → `16.8 ms`; idx 1151 / offset 1,816,059 → `57.1 ms`.** ⚠️ **~10× the offset ≈ ~10× the cost — LINEAR, and paid on every navigation.** ⚠️ **`center` stays flat at ~92 ms across all three, so this is `setSelectedRange` specifically, NOT the centring.** ---- ⚠️ **NEITHER IS A HANG — 59 ms and 57 ms are not beachballs** — ✅ **but both are the O(DOCUMENT) shape [SP-133] exists to remove**, ⚠️ **and `tvLen=1823873` in ONE `NSTextView` is why.** ⚠️ **The TextKit 2 migration did NOT eliminate this; ✅ the console confirms `TextKit 2` at construction AND at `rebuildStorage`, so this is TextKit 2 behaviour, not a silent downgrade.** ---- ⚠️ **DO NOT ASSUME THE CAUSE.** ⚠️ **The ~58 ms is attributed to AppKit ONLY BY SUBTRACTION (total minus our instrumented work); ✅ it has NOT been sampled.** ⚠️ **[I-0204] cost three wrong root causes read from code — ✅ this one must be `sample`d during sustained typing before any fix is designed.** | **Medium** | ⚠️ **Unassigned** — ✅ **candidate for [SP-133]/[EP-039]** | 🔵 **Open — ✅ MEASURED, ⚠️ CAUSE NOT SAMPLED.** |
## Currently: **seventeen records** — I-0147 (accepted limitation) + ⚠️ **I-0176, I-0177, I-0178, I-0180, I-0192, ⚠️ **I-0197 (Apple bypasses ScriviCore)**, ⚠️ **I-0200 (HIGH — the app is UNUSABLE after the load)**, ⚠️ **I-0201 (HIGH — a launch ARGUMENT silently prevents startup)**, ⚠️ **I-0202 (a latent `assumeIsolated` trap)**, ⚠️ **I-0203 (the Scene Navigator hides itself — ✅ TRIGGER FOUND: the world-unavailable banner; ⚠️ starvation DISPROVEN)**, ⚠️ **I-0205 (the world-unavailable banner itself)**, ⚠️ **I-0206 (~59 ms/keystroke + offset-linear `setSel`)**, — OPEN** + ✅ **I-0207 (HIGH — VERIFIED + ARCHIVED: the focus-click beachball)**  + ✅ **I-0182, I-0198, I-0199, I-0204 ARCHIVED 2026-09-14** (→ `Verified/`) + 🟢 **I-0181, I-0191, I-0194, I-0195 (Resolved - Not Verified)**

✅ **I-0193 VERIFIED 2026-09-10 (user-approved) and ARCHIVED** → [`Verified/Issue-verified-0191-0200.md`](Verified/Issue-verified-0191-0200.md) — ⚠️ **a NEW DECADE FILE.** ⚠️ **The header count also read `thirteen` against TWELVE rows before this edit** — ✅ **corrected to `eleven`, which matches the table and the enumeration.**

⚠️ **I-0193 and I-0194 were filed 2026-09-08 from T-0478's LIVE PASS on the real rig** — ✅ **both against SP-124.** ⚠️ **Neither was findable from the suite**: I-0193 needs a real blocking mount to show its 102 s, and I-0194 appears ONLY on the offline resolution path. ⚠️ **I-0195 was filed the same day from the SAME root architectural gap** — ✅ **no async path for world/project reads.** ⚠️ **I-0193 was the UNREACHABLE case (needed a timeout); I-0195 is the REACHABLE-BUT-SLOW case (needs progress).** ⚠️ **Fixing either without getting the read off the UI thread fixes neither.** ✅ **I-0193 is now VERIFIED and ARCHIVED — its `AsyncCall` machinery (`platforms/linux/src/AsyncCall.hpp`) is the OFF-THE-UI-THREAD PREREQUISITE I-0195 shares**, ⚠️ **so I-0195 should BUILD ON IT rather than re-derive it** — ⚠️ **but I-0195 remains OPEN: a timeout is not progress, and the determinate `files read / files to read` percentage the user specified is still unbuilt.**

✅ **I-0193 IS CLOSED — VERIFIED 2026-09-10 and archived.** ⚠️ **It took TWO passes.** ✅ **Build 34 (2026-09-09) fixed the scene-click path and UNBLOCKED T-0478's DoD item** — the writer-facing string was READ: *"This scene's objects are taking longer than expected to read — a world may be on a disconnected or unreachable volume. Nothing has been lost."* ✅ **It says `may be` and `Nothing has been lost` — the I-0115 discipline held: a wrong-but-confident status is what invites a writer to reach for destructive remedies.** ⚠️ **But that fix left the SECOND call site the Issue had NAMED FROM THE START untouched**, so `Project > Manage Worlds` still froze ~102 s to Force Quit. ✅ **`571fac1` closed it: `WorldsDialog::reload()` is async, and `EditorShell::writerFacingError()` no longer calls the core to name a world.** ✅ **User-verified on the real rig: the dialog OPENS with the share down.**

⚠️ **I-0192 came out of T-0477's S3 physical yank (2026-09-07) — and the INSTRUMENTATION DID NOT FIND IT.** ⚠️ **The probe was run on the WRONG MACHINE because the runbook never said which machine each command belonged to**, so it watched a Linux path on the MacBook and produced noise. ✅ **The user found the defect by PULLING THE DRIVE AND WATCHING THE APP.**

⚠️ **MY FIRST DIAGNOSIS OF I-0192 WAS WRONG AND IS WITHDRAWN (2026-09-07).** ⚠️ **I filed it as an APP-LAYER cached status, and repeated the user's reasonable suggestion that the placeholder dialog was merely reusing a name it already held.** ✅ **Reading the code disproved both**: the double-click performs a genuine `openObject` through the ABI and parses the name from the returned `objectJson`; ⚠️ **there is no app-side status cache**, and ✅ **`WorldStore::resolve` caches no verdict either** — it returns `available` only after reading and parsing `world.json`. ⚠️ **So the FILESYSTEM answered successfully for a volume that was physically gone, and every layer above correctly trusted a correct answer.** ⚠️ **Re-scoped `[Linux]` → `[ScriviCore]`.**

⚠️ **I also claimed "no probe in §4's table would have caught it." ✅ THAT WAS WRONG TOO** — ⚠️ **§4's held-FD probe questions exactly this**, and ✅ **2b had ALREADY measured the stronger form** (a held FD outliving `umount -l` + `losetup -D` entirely). ⚠️ **The finding was reachable by instrumentation; the instrumentation was simply pointed at the wrong machine.** ⚠️ **What is TRUE is narrower and still worth keeping: the OS was honest throughout, so no probe that questions MOUNT STATE would have flagged it** — ✅ **`feedback_live_pass_finds_what_suites_cannot` still applies, but as a claim about which QUESTION was asked, not about instrumentation being useless.**

### ⚠️ I-0192 — ✅ **HOW TO SETTLE IT** (the probe run)

⚠️ **The corrected diagnosis narrows the question; it does NOT answer it.** ⚠️ **Do not fold this into
T-0498 until it is MEASURED** — ⚠️ **fixing `resolve` from a reading of the code is exactly what I-0181's
history warns against** (three narrowings of one block, each made from inference).

✅ **The instrument now exists and builds on the rig**: `scrivi_world_probe`
(`ScriviCore/tools/scrivi_world_probe.cpp`, ⚠️ **Qt-free**, `-DSCRIVI_BUILD_TOOLS=ON` by default).
⚠️ **It did not exist when S3 ran, which is why S3 could not answer this.**

⚠️ **The run — 🐧 `oathkeeper`, one drive pull:**

| Phase | Command | ⚠️ What it settles |
| ----- | ------- | ------------------ |
| **BEFORE** | `scrivi_world_probe <project>` | The healthy baseline envelope |
| ⚠️ **IMMEDIATELY AFTER the yank** | ⚠️ **the same command, REPEATED every ~2 s** | ⚠️ **Does `resolve` still say `available` — and for HOW LONG?** |
| **AFTER a scene change** | same command | ⚠️ **Confirms the flip the user saw, from the ABI rather than from the screen** |

⚠️ **The decisive row is the second, and it must be SAMPLED, not snapshotted** — ✅ **the same reason
`volume-loss-probe.sh` streams:** a single reading cannot distinguish a decaying success from a
persistent one.

| ⚠️ If the probe shows… | ✅ Then |
| ---------------------- | ------- |
| `available` ⚠️ **DECAYING** to `unavailable` on its own | ✅ **The core is CORRECT** — the page cache was answering, and the only defect is that nothing re-asks promptly. ⚠️ **NOT T-0498's** |
| `available` ⚠️ **PERSISTING** indefinitely | ⚠️ **`resolve` is asserting PRESENCE it cannot prove** — ✅ **T-0498's `st_dev` check is then the fix for BOTH directions**, and I-0192 folds into it |

⚠️ **A successful `read` is NOT evidence of a mounted volume**, any more than ✅ **a surviving directory is
evidence of a deleted world** (I-0181). ⚠️ **Same wrong question, opposite sign.**

✅ **I-0191 was filed and fixed on 2026-09-07, the same day the user spotted the folder.** ⚠️ **It is the first Issue in this project found by the user noticing an ARTEFACT rather than a behaviour** — the app never misbehaved visibly; three empty directories simply sat in the repo root for three weeks. ⚠️ **Two of its seven ACs resolved differently than the Issue assumed**: the ⚠️ **test suite was cleared** (audited clean — RAII cleanup, absolute temp roots, no `chdir`), and ⚠️ **AC6's dangling-pointer suspicion did NOT survive the audit** — the `constData()` pattern is safe as written, so it was ✅ **RULED and DOCUMENTED rather than rewritten across 265 lines.** ⚠️ **Compare `feedback_evidence_before_attribution`: the suspected culprit was not the culprit, and checking beat assuming.**

✅ **I-0183, I-0184, I-0185 and I-0186 VERIFIED 2026-09-02 (user-approved) and archived** in the same
step → [`Verified/Issue-verified-0181-0190.md`](Verified/Issue-verified-0181-0190.md), ⚠️ **which OPENS
a new decade file.**

⚠️ **ALL FOUR came from ONE live pass (T-0496, SP-127), and NONE was caught by a suite.** ⚠️ **Two of
the four needed the USER TO CORRECT MY DIAGNOSIS before the real defect came into view** — I-0184
(I blamed a 360 px constant; the cause was the row layout **clipping**) and I-0185 (⚠️ **my first fix
made descending RECOVERABLE when the requirement was that it be IMPOSSIBLE** — and it passed every
test I had written for it). ⚠️ **Compare `feedback_live_pass_finds_what_suites_cannot` and
`feedback_prove_code_is_reached`: a green suite never means usable, and a test written from a wrong
diagnosis certifies the wrong thing.

✅ **I-0184 and I-0186 VERIFIED 2026-09-02 (user-approved) and archived** in the same step →
[`Verified/Issue-verified-0181-0190.md`](Verified/Issue-verified-0181-0190.md), ⚠️ **which OPENS a new
decade file.**

⚠️ **I-0186's root cause was a TESTING BLIND SPOT, not the code alone.** ⚠️ **Qt's no-theme fallback
made every offscreen check — and a screenshot produced as evidence — show a readable path that no real
user ever saw.** ✅ **The user found it by looking at their own screen.** ⚠️ **Compare
`feedback_live_pass_finds_what_suites_cannot`: a green suite never means usable, and this one was
green *because* it was headless.**

⚠️ **I-0183 is the most serious Issue in this file: it is DATA LOSS, and it was found by a LIVE PASS doing exactly what the sprint's own risk table said to test** — ✅ *"Relink accepting the wrong package → the CORE verifies `worldID`"* — ⚠️ **the mitigation was written, implemented, and is INSUFFICIENT, because a copy shares the `worldID`.** ⚠️ **A green suite never showed this** (`feedback_live_pass_finds_what_suites_cannot`).

⚠️ **I-0181 opens the new decade** and is ⚠️ **the first Issue in this project found by INSTRUMENTATION
rather than by use or by a suite.** ✅ **It was found BEFORE the surface that would have shown it was
written** — which is what *instrument-before-implement* is for.

⚠️ **It was RE-SCOPED TWICE in one day, both times by user ruling** — `[Linux]` → `[Cross]` →
⚠️ **`[ScriviCore]`.** ⚠️ **My "macOS is immune" claim did not survive a hand-specified mountpoint**;
⚠️ **then my framing as a REPORTING defect did not survive the observation that Linux has no world
surface to report through at all.** ✅ **"The app won't incorrectly represent the mount point until it
can correctly represent the mount point"** — ⚠️ **so this is a LATENT CORE defect, not a live one**,
and ⚠️ **it is NOT SP-124's to fix.**

⚠️ **NOT fixed**, and ⚠️ **must not be fixed from container evidence**: the container establishes the
CLEAN unmount case, and ⚠️ **the physical-yank case may differ.**

✅ **I-0179 VERIFIED 2026-08-30 and archived** → [`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md), ⚠️ **which CLOSES that decade file.**
⚠️ **The next Issue is I-0187.**

⚠️ **I-0180 is an APPLE defect found by reviewing the LINUX mirror.** ✅ **That is the port paying a
dividend back**: building the same surface a second time exposed a wrong label that had been shipping
on macOS since EP-031 unnoticed. ⚠️ **Worth remembering when the remaining four ports run.**

⚠️ **I-0179 was found by the user in SP-126's live pass**, in a message I had *just* rewritten to be
writer-facing — ✅ **the wording was right and the quoted string was wrong.** ⚠️ **Lesson: a row's
visible text is a PRESENTATION.** Recovering data by parsing it back apart works until the
presentation changes, and here it never worked at all.

⚠️ **All three were found by the USER on the REAL RIG (T-0476, 2026-08-29)** — ⚠️ **the first time the
Linux app had ever run on real hardware**, and ⚠️ **none of them was findable by any suite**: they are
about what survives a QUIT, which no test exercises.

✅ **They are ONE gap with three symptoms, not three bugs.** ⚠️ **I-0178 (multi-project) is the parent** —
Apple solved all three together in **EP-018**, whose per-window `ProjectSession` + `OpenProjectRegistry`
is what "restore what was open" and "restore geometry" both hang from. ⚠️ **A Linux equivalent is a
STRUCTURAL rework of `ScriviWindow`/`EditorShell`, and wants its OWN Epic.**

⚠️ **The user ruled these do NOT block T-0476's verification** — they are gaps in scope never claimed,
not failures of what was built.

✅ **I-0171 VERIFIED 2026-08-29 and archived** → [`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md).
⚠️ **It was fixed by SP-125 but OWNED by SP-122** — verified in the same step SP-125 closed.

✅ **SP-125's three Issues were settled 2026-08-28 in the same step its five Tasks were verified**
(`feedback_archive_on_close`):

- ✅ **I-0173** (elided relationship labels) — **Verified** → [`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md).
  ⚠️ **Found by the LIVE PASS; all 571 ctests and 23 smoke checks were green with it present.**
- ✅ **I-0175** (a synthetic-input driver typed into a real manuscript) — **Verified**, same file.
  ⚠️ **My process defect, not the app's**; repaired byte-for-byte.
- ⚠️ **I-0174 CLOSED as NOT A DEFECT** → [`Closed/Issue-closed-0174.md`](Closed/Issue-closed-0174.md).
  ⚠️ **My diagnosis was wrong and the user corrected it**: the "unexplained" cache write was a second
  project's characters propagating through a **shared world**. ✅ **Opening a project is not a risk.**

✅ **I-0172 was Verified 2026-08-25 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md), which **opens a new
decade file** (the previous closed at I-0170).

⚠️ **I-0172 was verified by COMPILATION plus user approval, not by exercising the popover** — the fork
popover appears only when redoing into a branch point, which SP-122 never hit. ⚠️ **If a sizing
regression appears in that popover, I-0172's change is the first thing to suspect.**

⚠️ **I-0171 was opened 2026-08-25 by SP-122's T-0468** and is the first Issue of the new decade.
⚠️ **It was found by RUNNING the Linux leg, not by reading the `.dockerignore`** — SP-121 added that file
and its own sprint never re-ran a cached container build against a second build directory.

✅ **I-0169 + I-0170 were Verified 2026-08-24 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0161-0170.md`](Verified/Issue-verified-0161-0170.md), which that pair **closes**.
⚠️ **The next Issue is I-0171 and opens a new decade file.**

⚠️ **Both came from SP-120's live click-through; neither from any suite** — which now holds for **22
consecutive Issues** across SP-118, SP-119 and SP-120. ⚠️ **I-0169 was the writer's FIRST instinct**
(the sources card had no route to the Detail Sheet, using a hook that already existed and was never
called); ⚠️ **I-0170 was a surface quietly UNDER-REPORTING the graph** — every field present and
populated, and still not true.

✅ **I-0162 – I-0168 were Verified 2026-08-24 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0161-0170.md`](Verified/Issue-verified-0161-0170.md).

⚠️ **All seven came from SP-119's live click-through. None was found by any suite.** ⚠️ **Six were
data-loss routes into a single surface** — the Object Detail Sheet — reachable by ejecting a drive or
navigating away at six different moments.

**What the table cannot express:**

- ⚠️ **I-0161 took THREE attempts and is the sprint's clearest lesson in diagnosis order.** Attempt 1
  scrolled at click time (wrong: raced the highlight). Attempt 2 fixed that correctly but ⚠️ **was never
  compiled into the macOS build** — the edit reached one of two platform call sites. ⚠️ **Claude spent a
  round explaining the behaviour of code that did not run**, exactly as I-0151 was caused by a comment
  asserting behaviour never checked against the source.
- ⚠️ **THE RULE: prove the new code is REACHED before explaining why it behaves oddly.** One log line, or
  one grep for call sites, would have replaced a whole round of theory. ⚠️ **"It didn't change anything"
  should first be read as "it isn't running", not as "it ran and was wrong."**
- ✅ **Temporary `SCRIVI-DIAG` logging is what settled it** — and was removed once it had. Instrumenting a
  path is cheaper than a third hypothesis.
- ⚠️ **I-0158/I-0159 are one mistake with two faces: I hand-rolled a list.** A `VStack`/`ForEach` meant
  reimplementing selection, the highlight and right-click targeting — each attempt wrong in a new way —
  and ⚠️ **`SceneNavigatorView` was already doing it correctly with `List(selection:)` in the same
  directory.** Switching to `List` fixed selection and broke layout; the answer was to take the selection
  semantics and keep the app's existing scroll structure. ⚠️ **The user's question — "a Swift standard
  List View handles all this automatically… which makes me wonder why it is so hard for you" — is the
  right one**, and the answer is that I built new machinery instead of looking at what the app already had.
- ⚠️ **THREE defects this sprint were "an existing correct pattern the new code did not follow"**: I-0155
  (`ObjectCardModel.rename` re-read before patching), I-0157 (I-0132 ruled selection the source of truth),
  I-0158 (`SceneNavigatorView` already used `List(selection:)`). ⚠️ **All three rules were written down,
  in this repo, before the code that violated them was typed.**
- ⚠️ **I-0155 is the most serious defect of the sprint, and it was reported as a hedge.** The user wrote
  *"It isn't necessarily a defect. More like an unintended consequence… Maybe there is a defect here after
  all."* ⚠️ **It was silent data loss** — a saved note reverting a saved rename. **The uncertainty in a
  report is not a measure of its severity**, which is the same lesson as I-0148 and I-0154, now three
  times in this Epic.
- ⚠️ **I-0155 and I-0157 share a shape: an existing correct pattern that the new surface did not follow.**
  `ObjectCardModel.rename` already re-read before patching; I-0132 already ruled selection the source of
  truth. ⚠️ **Both rules were written down, both were violated by code added days later.** Grepping for
  "how does the app already do this?" would have caught both — the same discipline as the
  derive-never-restate rule, applied to behaviour instead of to lists.
- ⚠️ **I-0151–I-0154 were ALL found by the SP-118 live click-through**, and none by any suite. ⚠️ **The
  green run had asserted edge creation, duplicate rejection, both-endpoint visibility and pending
  presentation** — every one of which held up. **What no test covered was whether a writer could reach any
  of it**, which is `capability_without_surface` for the third time in this Epic.
- ⚠️ **I-0151's cause was a COMMENT ASSERTING A FALSEHOOD.** I wrote *"`openObject` accepts '' and resolves
  it"* next to the line that passed `""`, and never opened `ObjectStore.cpp` to check. ⚠️ **A confident
  comment is not evidence**, and writing one is how an unchecked assumption gets laundered into an
  apparent finding — the same failure as I-0150's misattribution, in a different medium.
- ⚠️ **I-0152 is the one Claude got wrong twice.** Told the writer saw a raw ID, Claude confirmed the empty
  title and concluded *"not a display bug"* — answering **why the data was empty** instead of **what the
  writer was shown**. The user's correction was the point: the Navigator already solved this, so two
  surfaces disagreed about one scene's name and the worse answer won.
- ⚠️ **"The Lantern Foxes" is NOT a defect** — checked and closed. The stored edge is
  `chronicle --appears-in--> scene`, so *"appears in"* from the chronicle's end and *"features"* from the
  scene's end are **the same edge read from opposite endpoints** (Doc 1 §5.2), and `ObjectCard` passes
  `label: edge.label` straight through without recomputing. ✅ **Both displays are correct.**
- ⚠️ **I-0150 was found by the user REFUSING A PLAUSIBLE STORY.** Claude read a timestamp, concluded
  *"you reopened Scrivi"*, and wrote a detailed accounting on that basis. ⚠️ **The user simply said he had
  not** — and the real cause was Claude's own test command. ⚠️ **The failure mode was reaching for the
  explanation that did not implicate my own actions**, and the evidence was in a file I had already been
  told to update (`TEST_HOST` in `project.pbxproj`).
- ⚠️ **I-0150 changes what "safe to test" means on this project.** `xcodebuild test` is **not** a read-only
  operation: it is an app launch with full access to the writer's real projects through saved bookmarks.
  ⚠️ **There is deliberately NO test that flips the guard off to prove the projects reopen** — that
  negative control would re-enable the damaging behaviour on a real machine with real bookmarks. The
  evidence is a before/after checksum of all 220 files, not a reproduction of the harm.
- ⚠️ **I-0149 is the SIXTH EP-034 defect found by use rather than by tests** (I-0137, I-0142, I-0146,
  I-0147, I-0148, I-0149) — ⚠️ **and the first found by a user asking whether the work had actually
  happened.** The suite was green, the binary contained the fix, and the fix did nothing.
- ⚠️ **The lesson is narrower and sharper than "test more".** T-0441 had a drifted fixture, a negative
  control, and a passing assertion that the repair worked. ⚠️ **All of it tested the REPAIR and none of it
  tested the TRIGGER.** A test that calls `load()` to check that `load()` repairs is a tautology wearing a
  fixture; the missing test was *"open a project and touch nothing else."*
- ⚠️ **"On open" is an EVENT, not a function.** The ruling named the event; the implementation picked a
  function that seemed adjacent to it. ⚠️ **When a ruling names a moment, the test must reproduce that
  moment** — not a call that usually accompanies it.
- ⚠️ **A stale test binary nearly hid the fix too.** The Xcode app build reconfigures the shared `build/`
  directory with `SCRIVI_BUILD_TESTS=OFF`, so `cmake --build` silently left a 28-minute-old
  `ScriviCoreTests` in place and the new tests reported *"No tests ran"* — which reads like a filter typo,
  not a stale binary (`project_linux_container_tests_off` is the same class on Linux).

- ⚠️ **I-0147 is a KNOWN LIMITATION, not a defect awaiting a fix** (user ruling, option 1). For up to 60 s
  after an interrupted world write, the world is unwritable and its `.partial` unreclaimable, because the
  dead writer's lock is not yet stale and the sweep only runs after a successful acquire. It **self-heals**
  and loses no data. ⚠️ **A regression test ASSERTS this behaviour** — if someone later makes `acquire`
  break fresh locks, it fails and forces the locking-model conversation rather than letting it happen by
  accident (the lesson of I-0144).
- ⚠️ **The eventual UI must never present the 60 s wait as an error** — it is a retryable state.
- ⚠️ **I-0148 is the FIFTH defect in EP-034 found by use rather than by tests** (I-0137, I-0142, I-0146,
  I-0147, I-0148) — and the first the user reported **without recognising it as a defect**, folded into an
  otherwise positive report. ⚠️ **A satisfied user is not a green suite**: the observation mattered more
  than the verdict attached to it.
- ⚠️ **Three of SP-116's six were found by no suite at all**: I-0143 by reading the code D7 was about to
  modify, I-0144 by looking for a caller to mirror, and **I-0146 by physically pulling a USB drive**.

---

## ✅ SP-115 — all six Issues Verified 2026-08-20

| Issue | Sev | Task | Archive |
| ----- | --- | ---- | ------- |
| **I-0137** | **High** | T-0419 | [`Verified/Issue-verified-0131-0140.md`](Verified/Issue-verified-0131-0140.md) |
| I-0136 | Medium | T-0420 | same |
| I-0139 | Medium | T-0421 | same |
| I-0135 | Low | T-0422 | same |
| I-0138 | Low | T-0423 | same |
| **I-0142** | **High** | T-0425 | [`Verified/Issue-verified-0141-0150.md`](Verified/Issue-verified-0141-0150.md) |

⚠️ **I-0137 was verified on the REAL RIG** with the drive ejected — the check a passing suite genuinely
cannot substitute for.

⚠️ **I-0136 is Verified at the CORE ONLY.** Nothing in Scrivi surfaces `unsupportedWorldFormatVersion`, so
a writer opening a too-new world still sees *"unavailable"* with **no explanation**. The core refuses
correctly; **the writer-facing half does not exist** — `project_capability_without_surface` inside the very
sprint that fixed four other instances. **Owed a surface in a later sprint.**

⚠️ **I-0142 was found by the USER, not a suite** — and its unseen half (**renaming any world object
failed**) was worse than the reported symptom.

---

*Last Updated: 2026-08-24, twenty-sixth pass (**I-0162 – I-0168 ✅ VERIFIED (user-approved) and ARCHIVED**
at SP-119 close → the new `Verified/Issue-verified-0161-0170.md`. Open Issues **7 → 0**; I-0147 remains an
Accepted limitation, not open work. ⚠️ **All seven came from the live click-through; six were data-loss
routes into one surface.** Next available Issue: **I-0169**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-fifth pass (⚠️ **I-0168 FILED — the Scene Inspector bypassed T-0452's
guard.** ⚠️ **The guard was in the wrong PLACE**: the host owns the history and the inspector asks the
host, so the sheet was never consulted. ⚠️ **T-0452 swept the four exits that originate inside the sheet
and could not see the one that originates outside it.** ✅ Fixed by moving the decision to a single owner
rather than adding a fifth check; ✅ **every history mutation swept.** ⚠️ **Sixth data-loss route in this
Epic** — the user has now found all six by ordinary use. Open Issues: **7**. Next available Issue:
**I-0169**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-fourth pass (⚠️ **I-0167 FILED — the ✕ discarded unsaved edits with no
prompt and no way to revert.** ⚠️ **Third route into this Epic's data loss**, and the only one a writer
triggers with an ordinary click. ⚠️ **Back/forward and related-list navigation shared the exposure** and
were fixed in the same pass rather than left for a later report. ✅ **Cancel + Save/Discard prompt**, to the
user's own design; ⚠️ **explicitly NOT undo** per their ruling. Open Issues: **6**. Next available Issue:
**I-0168**; Task: **T-0453**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-third pass (⚠️ **I-0166 FILED — cold-opening an object with its world
away showed a raw error code**, R9 violated in the case R9 exists for. ⚠️ **I-0165's fix covered only the
already-loaded sheet**, and its own comment claimed there was "nothing to show" when history carried the
object's name all along. ✅ Fixed with a `worldUnavailable` accessor mirroring the existing
`isWorldPending` idiom. ⚠️ **Fourth defect in one chain, each found by ejecting the drive at a different
moment.** Open Issues: **5**. Next available Issue: **I-0167**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-second pass (⚠️ **I-0165 FILED — a REGRESSION FROM I-0162'S FIX.**
Ejecting the drive replaced the whole Detail Sheet with a raw ScriviError and ⚠️ **discarded unsaved
edits** — R9 violated outright. The new `worldRevision` reload hit `load()`'s catch branch, which had
always been allowed to blank the sheet because it previously only ran on navigation. ✅ Fixed: a failed
re-read keeps the object and lets the read-only banner explain the outage. ⚠️ **Found by the re-test of
the very fix that caused it.** Open Issues: **4**. Next available Issue: **I-0166**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-first pass (⚠️ **I-0164 FILED — OPEN, needs a ruling.** An asset already
in a world **cannot be attached** to an object, and the only workaround — re-importing the same file —
⚠️ **silently orphans the first assetID**, because both bytes and sidecar are named after the FILENAME.
✅ **Proven by test**: one asset on disk, new ID listed, first ID unresolvable. ⚠️ **S11 missed it because
it enumerated FIELDS, not OPERATIONS** — `listAssets` was marked "not surfaced" without asking what a
writer would use it for. ✅ **The T-0447 chain itself is PROVEN WORKING** — the Tintagael location's image
imports, links, indexes and displays correctly. Open Issues: **3**. Next available Issue: **I-0165**.
Prior note follows.)*

*Last Updated: 2026-08-24, twentieth pass (⚠️ **I-0163 FILED — an image on disk in a world was invisible
to the app.** ⚠️ **A derived cache written before a field exists never rebuilds itself**, and T-0446's
tests could not see it because they always create their index with the current build. ✅ Fixed with an
index `generation` marker; ⚠️ **bump it when adding an entry field.** ⚠️ **Claude chased a phantom
failure for several rounds — the test had been passing and the binary was stale**
(`feedback_prove_code_is_reached`, third occurrence). Open Issues: **2**. Next available Issue: **I-0164**.
Prior note follows.)*

*Last Updated: 2026-08-24, nineteenth pass (⚠️ **I-0162 FILED — an ejected drive reported the writer's
image as DAMAGED rather than absent**, found by the user's SP-119 step-7 click-through. ⚠️ **Two causes:
the sheet never reloaded on a world-availability change** (`session.worldRevision` already existed and the
inspector cards already watched it — ⚠️ **the fourth "existing pattern not followed" since SP-118**), and
the outage branch was load-time only. ⚠️ **Claude's first two hypotheses were wrong**; the cause was
settled by probing the core (`loadAllVisible` → count=0 for an unavailable world). Open Issues: **1**.
Next available Issue: **I-0163**. Prior note follows.)*

*Last Updated: 2026-08-23, eighteenth pass (**I-0149 – I-0161 ✅ VERIFIED (user-approved) and ARCHIVED in
the same step** at SP-118 close → `Verified/Issue-verified-0141-0150.md` and the new
`Verified/Issue-verified-0151-0160.md`. Open Issues **13 → 0**; I-0147 remains an Accepted limitation, not
open work. ⚠️ **All thirteen came from the live click-through and none from any suite.** ⚠️ **Four were one
failure — an existing correct pattern the new code did not follow.** Next available Issue: **I-0162**.
Prior note follows.)*

*Last Updated: 2026-08-22, seventeenth pass (⚠️ **I-0159: the related list LOOKED like it had lost rows** —
a nested `List` inside the sheet's ScrollView hid 5 of Myton's 8 behind an invisible second scroll;
⚠️ **the USER diagnosed it.** I-0160: ⚠️ **I-0155 had been fixed in one direction only.** I-0161: navigator
reveal for navigation from another surface, ⚠️ **carefully distinguished from the reveal I-0132 removed.**
Open Issues: **12**. Next available Issue: **I-0162**. Prior note follows.)*

*Last Updated: 2026-08-22, sixteenth pass (⚠️ **I-0155 FILED — SILENT DATA LOSS**: a Detail Sheet save
patched a snapshot from sheet-open, reverting a Scene Inspector rename. ⚠️ **Reported by the user as
possibly not a defect at all.** Fixed in three parts, incl. per-field conflict resolution so the fix does
not reverse the loss. I-0156: rows had no selection. I-0157: scene navigation bypassed I-0132's
selection-is-truth ruling. Open Issues: **9**. Next available Issue: **I-0158**. Prior note follows.)*

*Last Updated: 2026-08-22, fifteenth pass (⚠️ **I-0151–I-0154 FILED AND RESOLVED — all four found by the
SP-118 LIVE CLICK-THROUGH, none by any suite.** ⚠️ **I-0151 broke navigation to every world-scoped object**
and was caused by a comment asserting a falsehood I never checked. ⚠️ **I-0152 showed the writer a raw
scene ID** where the Navigator already knew a useful name — ⚠️ **Claude dismissed it once and the user
was right to reject that.** I-0153: scene rows were a dead affordance. I-0154: no right-click highlight.
✅ **"The Lantern Foxes" checked and CLOSED as correct** — opposite endpoints of one edge. Open Issues:
**6**. Next available Issue: **I-0155**. Prior note follows.)*

*Last Updated: 2026-08-22, fourteenth pass (⚠️ **I-0150 FILED AND RESOLVED — `xcodebuild test` launches the
real app and reopened the user's ACTUAL PROJECTS.** ⚠️ **This, not a user launch, is what modified
`the-twisted-remains-of-myself.scrivi`; Claude had misattributed it to the user and was corrected.**
✅ Fixed at the choke point in `restoreOpenProjects()`; ✅ **verified by checksums of 220 files across three
full test runs — byte-identical**. ⚠️ **`pgrep Scrivi` never protected against this.** Open Issues: **2**
(I-0149, I-0150). Next available Issue: **I-0151**. Prior note follows.)*

*Last Updated: 2026-08-22, thirteenth pass (⚠️ **I-0149 FILED AND RESOLVED — found by the USER asking
whether the migration had actually occurred**, after SP-118 reported green. ⚠️ **T-0441 reconciled on READ,
not on OPEN** — the repair lived in `RelationTypeStore::load()`, which a project open never calls; the real
rig opened a drifted project with the fix in the binary and changed nothing. ✅ **Fixed in
`ProjectOpener::open`** as repair pass (e); ✅ **negative control run** (the new test fails against
T-0441-as-shipped); ✅ **verified against a copy of the user's real project**. ⚠️ **T-0441 is NO LONGER
"Implemented"** on its own — it is complete only with I-0149. `ctest` **561/561**. Open Issues: **1**
(I-0149, Resolved - Not Verified). Next available Issue: **I-0150**. Prior note follows.)*

*Last Updated: 2026-08-21, twelfth pass (✅ **I-0148 VERIFIED (user-approved) and ARCHIVED in the same
step.** ⚠️ **It was found by the user's live click-through and reported as an OBSERVATION, not a
complaint** — the fifth defect in EP-034 found by use rather than by tests. **Open Issues: 0**; I-0147
remains an Accepted limitation. Next available Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, eleventh pass (⚠️ **I-0148 FILED AND RESOLVED — found by the user's LIVE
CLICK-THROUGH of SP-117**, and ⚠️ **reported as an observation, not a complaint**: `.disabled()` does not
make a `TextEditor` read-only, so Notes stayed editable beneath a "read only" banner. ✅ **Never a
write-safety bug** — Save is hidden when read-only — ⚠️ **but typing during an outage was silently
discarded on navigation**, since `load()` overwrites the draft. **User ruled: disable it**, for simplicity
and consistency over draft retention. Notes now renders as selectable text when read-only. Next available
Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, tenth pass (✅ **SP-116's SIX ISSUES VERIFIED (user-approved) and ARCHIVED in
the same step** → `Verified/Issue-verified-0141-0150.md` (`feedback_archive_on_close`). ⚠️ **I-0147 remains
here as an ACCEPTED limitation** — deferred to the network-worlds design, with a regression test asserting
it. **Open Issues: 0.** Next available Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, ninth pass (⚠️ **I-0147 FILED AND ACCEPTED as a known limitation** (user ruled
option 1): for up to 60 s after an interrupted world write the world is unwritable and its `.partial`
unreclaimable, because the dead writer's lock is not yet stale and **the sweep only runs after a successful
acquire**. ⚠️ **Found by the tidy end-to-end rig run** — drive pulled, reattached quickly, next write
refused `worldLocked`, **2.9 GB orphan retained**. ✅ **Both halves verified** (fresh lock → refused; past
60 s → acquired **and swept**). ⚠️ **My earlier staged-orphan test passed only because it omitted the
matching fresh lock** — a setup subtly easier than reality; **fourth defect this Epic found only by live
use**. **Deferred to the network-worlds design**, which must revisit "exactly one winner" anyway.
Open Issues: **0** (I-0147 is Accepted, not open). Next available Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, eighth pass (✅ **I-0146 ASSIGNED to SP-116 (T-0433) and RESOLVED** by user
ruling. `WorldLock::sweepAbandonedPartials()` reclaims abandoned `*.partial` files whenever the lock is
acquired. ⚠️ **Swept on EVERY successful acquire, not only after breaking a stale lock** — the rig showed
the lock file and the partial are orphaned TOGETHER, so the next writer acquires cleanly and never reaches
a break path; sweeping only on a break would have missed the exact case this Issue was filed for.
⚠️ **Verified on real hardware**: 459 MB orphan on the USB volume reclaimed by a normal import, 476 MiB →
12 MiB, real assets and `myton.json` untouched. Tests **551/551** (+4), ⚠️ **proven non-vacuous** —
disabling the sweep fails two. **Open Issues: 0.** Next available Issue: **I-0147**. Prior note follows.)*

*Last Updated: 2026-08-21, seventh pass (⚠️ **I-0146 FILED — found by the LIVE RIG PASS, not by a suite.**
Pulling a real USB drive mid-import left a **459 MB `.partial` orphan** inside the shared world: the
cleanup in `copyFileInBlocks` cannot run when the failure IS the volume vanishing. ⚠️ **`list_assets`
cannot see it, so nothing in Scrivi will ever reclaim it.** ✅ **The rest of the abort behaved correctly** —
heartbeat detected the loss, transfer aborted, no destination file, existing assets byte-identical, stale
lock breakable after 60 s. **Fix is the user's own stale-lock sweep**, which SP-116 did not implement.
Open Issues: **1** (I-0146). Next available Issue: **I-0147**. Prior note follows.)*

*Last Updated: 2026-08-21, sixth pass (**I-0144 🟢 Resolved - Not Verified** — every world-package write
path now takes the lock via `WorldWriteGuard`, ⚠️ **inert for project writes so there is no branch to
forget**. ⚠️ **One deliberate exception recorded**: `ObjectIndex::loadWorldIndex`'s rebuild stays unlocked
because `WorldLock` is NOT REENTRANT and `save`/`remove` reach it while holding the lock — a guard there
would fail against itself and skip the rebuild. It is idempotent; the real fix is a reentrant lock, which
belongs with the network-worlds design. **Open Issues: 0.** Next available Issue: **I-0146**. Prior note
follows.)*

*Last Updated: 2026-08-21, fifth pass (**I-0145 FILED — 🟢 Resolved - Not Verified.** ⚠️ **Pre-existing and
shipped**: `AssetStore::remove` deleted the sidecar first and discarded both results, so a half-failed
delete stranded **bytes with no sidecar — invisible to `list` and unfindable by any future `remove`**,
unreclaimable for the life of the package, with `deleted: true` returned regardless. ⚠️ **D6 raises its
severity**, since the junk now lands in a SHARED world. Found by **self-review**; ⚠️ **no test caught it**.
✅ **Fixed in T-0426** (binary deleted first, both failures reported). ⚠️ **A sibling defect was
deliberately NOT filed** — `ObjectKindScope`'s duplicate-key trap was written and fixed inside this sprint
and never shipped. Open Issues: **1** (I-0144). Next available Issue: **I-0146**. Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (✅ **I-0144 ASSIGNED to SP-116** by user ruling → **T-0431**;
⚠️ **it is a High-severity data-loss risk, not an asset defect** — every object write into a shared world
is unserialised. Open Issues: **1**, now assigned. Next available Issue: **I-0145**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**SP-116 IMPLEMENTED — I-0140, I-0141, I-0143 all 🟢 Resolved -
Not Verified.** ⚠️ **I-0140 and I-0143 were each proven non-vacuous by reverting the fix** and watching the
tests fail. ⚠️ **I-0144 FILED (High, unassigned)**: `WorldLock` has **no production caller** — world-package
object writes are unserialised and have been since they shipped, so two projects sharing a world can lose
each other's edits silently. Found while implementing T-0426, looking for a caller to mirror; **no test
would have caught it**, since a missing lock is invisible single-threaded. ⚠️ **Not fixed in SP-116** — it
touches every object write path, not assets. Open Issues: **1** (I-0144). Next available Issue: **I-0145**.
Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**SP-116 ACTIVATED** — all three open Issues are now assigned to
an **active** Sprint, not a planned one; Sprint fields marked 🟡. ⚠️ **None is Resolved** — activation is
not progress, and Claude may never mark an Issue Verified regardless
(`feedback_verification`). Next available Issue: **I-0144**. Prior note follows.)*

*Last Updated: 2026-08-21 (**I-0143 FILED at SP-116 planning** — ⚠️ `scrivi_list_assets` concatenates
its JSON with **no escaping** (`scrivi_c_api.cpp:1330-1341`), while every sibling envelope uses `JsonDoc`.
⚠️ **Found by reading the code D7 modifies, not by a test and not by the design doc** — and D7 is precisely
what makes it reachable, since **T-0427 puts a filesystem path into that array**. ✅ **User ruled: file it
AND fix it in SP-116** (T-0428), keeping T-0424's file-don't-fix-silently precedent while refusing to ship
a corruption path the same sprint could prevent. ⚠️ **The restating summary table below the main table was
REPLACED** with only what the table cannot express (P7). Open Issues 2 → **3**, all SP-116. Next available
Issue: **I-0144**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115's six Issues ✅ VERIFIED by the user and ARCHIVED in the same step** —
I-0135–I-0139 → `Verified/Issue-verified-0131-0140.md`, **I-0142 → a new decade file
`Issue-verified-0141-0150.md`.** Open Issues 8 → 2 (**I-0140, I-0141** — filed for SP-116, unfixed by
design). ⚠️ **I-0137 verified on the real rig, drive ejected.** ⚠️ **I-0136 verified at the CORE ONLY — its
writer-facing surface does not exist and is owed.** Suites: ctest **525/525** · interop **103/103** · app
**BUILD SUCCEEDED**. Next available Issue: **I-0143**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 implemented — all five Issues 🟢 Resolved - Not Verified**, and
⚠️ **I-0140 + I-0141 FILED by T-0424** (restated-kind-list class, occurrence eight → **SP-116**, cured by
D5). Suites: `ctest` **524/524** (was 520) · macOS interop **103/103 in 10 suites** (was 99) · app
**BUILD SUCCEEDED**. ⚠️ **I-0137 still needs the REAL-RIG check** — drive ejected — before it can be
Verified. Open Issues 5 → 7. Next available Issue: **I-0142**. Prior note follows.)*

*Last Updated: 2026-08-20 (**All five open Issues ASSIGNED to SP-115** 🟡 Active under **EP-034** — one
Task each, T-0419–T-0423. ✅ **Two carried rulings recorded**: **D9 = A** for I-0137
(`lastKnownPackagePath`, distinctly named; `packagePath` NOT widened) and **Q-b** for I-0139 (**patch the
control** — the Detail Sheet does **not** replace the inline editor, so it is a real fix). ⚠️ **I-0140 and
I-0141 to be FILED by T-0424.** Next available Issue: **I-0142** after that filing. Prior note follows.)*

*Last Updated: 2026-08-19 (**T-0390 + T-0418 filed five Issues — I-0135…I-0139.** The live pass on the
real USB rig **passed steps 3, 4 and 5**: ⚠️ **AC23's no-intervention clause HELD** — reattaching the drive
restored every card with no click, no menu, no relaunch. Step 1 confirmed **all ten world kinds
round-trip** (the four directories absent since before SP-104 were created on demand); ⚠️ **`source` could
not be created — no UI exists, the known EP-034 gap.** Step 2 was **blocked**: relating from an object card
opens an editor whose exit is labelled "Revert" (I-0139). Findings: **I-0137 (High)** — AC24's refinement
**cannot fire on real hardware**; **I-0138** — disabled-but-unexplained removal; **I-0139** — the editor
exit. Next available: **I-0140**. Prior note follows.)*

*2026-08-18, fourth pass (✅ **I-0132 VERIFIED (user-approved) and re-archived** — both
halves, on an extended live click-through: *"I clicked about a lot and saw no missed clicks or focus
changes."* It took **four** attempts; the first three misdiagnosed it as a first-responder race and
each made the failure rarer rather than fixing it. ⚠️ **The user stopped the fourth before it was
written** — I was about to add an `NSEvent` monitor, reaching further below SwiftUI to win a fight
created by reaching below it in the first place — and redirected to the actual question: *what is the
source of truth, and does it propagate through the View hierarchy?* **The real defect was a one-shot
`navigateToSceneID` trigger**, not responder arbitration: re-selecting the same scene wrote an
unchanged value and SwiftUI coalesced the update away. macOS now uses the selection-as-source-of-truth
shape iOS already had. A **user-prompted loop audit** then replaced a fragile value-equality guard
with explicit echo suppression, plus **two regression tests proven non-vacuous**. Interop **95/95
macOS arm64**. Active count: **2** (I-0133 Resolved-Not-Verified, I-0134 Open). Prior note follows.)*

*2026-08-18, third pass (⚠️ **I-0132 RETURNED FROM VERIFIED — I archived it on a claim
that was not true.** The user verified focus changing **on app launch**, and said so explicitly; I
recorded that as verifying **click-to-focus** as well. Clicking a scene still left focus in the
navigator. **Cause was a responder race, not a missing call:** `takeFocus` ran
`makeFirstResponder` synchronously from inside `onTapGesture`, and the `NSTableView` backing SwiftUI's
`List` reclaimed first responder while finishing its own mouse-down. Launch had no competing responder
change, which is exactly why the two cases diverged — **the evidence I verified against and the
failing case were different code paths.** Second fix defers the transfer one runloop pass. The
**reveal half stays verified** and remains archived. **Lesson recorded:** when a fix has two halves,
verify each half against its own trigger — a verification of one is not evidence for the other.
Active count: 2 → **3**. Prior note follows.)*

*2026-08-18, later same day (**I-0131 + I-0132 ✅ Verified (user-approved) and
archived** to the new `Verified/Issue-verified-0131-0140.md` decade file, and removed from this file
in the same step. **I-0133 ruled and resolved:** the user chose *delete Apple's dead state, leave
Linux alone* — the property, its `loadAll` parameter, the write, the clear and the `ProjectSession`
plumbing are gone, each site commented so the omission reads as deliberate; ⚠️ **the schema field
stays** because Linux consumes it. ⚠️ **Ruling I-0133 surfaced a finding the original report missed,
now filed as I-0134 (🔴 Open):** Linux applies the scroll fraction *after* `centerCursor()`,
deliberately overriding it — so **Apple and Linux now disagree about what "restore where I was"
means.** Deliberately **not** settled inside a dead-code cleanup: it changes shipped, VNC-verified
EP-022 behaviour and belongs to EP-026 parity. **BUILD SUCCEEDED**, interop **93/93 macOS arm64**.
⚠️ **Active count is now 2** — I-0133 (Resolved - Not Verified) and I-0134 (Open). The prior note's
"Active count: 10 → 13" was already stale before this pass: those Issues had been verified and
archived without this line being updated. Prior note follows.)*

*2026-08-18 (**I-0132 both halves now 🟠 Implemented - Not Verified.** ⚠️ **The
reveal-on-selection-change half was REMOVED, not tuned** — the user's re-test found it scrolled the
navigator "a little bit up or down" on **every** click, because `scrollTo` **re-anchors an
already-visible row** rather than no-opping as my comment had claimed. Reveal now fires **`onAppear`
only**, which is the one moment it is needed (restore sets the selection before the view exists).
The **focus half is implemented**: `navigate(to:)` calls `loader.takeFocus()`, so a click or Return
hands the keyboard to the manuscript and the caret is visible. ⚠️ **Accepted trade, user-ruled:**
this ends arrow-key list browsing after the first click — *"Arrow browsing isn't strictly necessary.
Mouse Wheel and Trackpad Scrolling are still available."* **Tab-as-focus-advance is no longer needed
for this Issue.** Also filed **T-0417** (Scene/Chapter boundary navigation) — adopted into SP-102,
shipping as menu items because ⚠️ **no free macOS key combination exists.** **BUILD SUCCEEDED**,
interop **93/93 macOS arm64**. Prior note follows.)*

*2026-08-17, later same day (**I-0114–I-0117 ✅ Verified (user-approved) and archived** to
`Verified/Issue-verified-0111-0120.md` in the same step — verified live during the SP-102 / T-0415
world-availability runs, which exercised those exact surfaces. ✅ **They are now usable as evidence for
SP-100's AC pass**, which the prior note said they were not. **Also filed and fixed the same day:
I-0123–I-0129**, all from the user's live SP-102 runs. Active count: 10 → **13**. Prior note follows.)*

*2026-08-17 (**SP-106 closed — I-0121 and I-0122 ✅ Verified and archived** to the new
`Verified/Issue-verified-0121-0130.md` decade file, and their full entries removed from `Issue-backlog.md` in
the same step. Neither was ever listed in this file — both were tracked in `Issue-backlog.md` and the SP-106
sprint record. **This file is unchanged otherwise: the same 10 `Resolved - Not Verified` Issues remain
active**, including I-0114–I-0117, which are **not** evidence for any EP-031 AC until verified. Prior note
follows.)*

*2026-08-15 (docs cleanup — 48 verified Issues archived to decade files, 4 closed Issues
archived; 6 stale full entries (I-0064, I-0067–I-0071) reconciled against their authoritative table rows.
10 `Resolved - Not Verified` Issues remain active.)*
