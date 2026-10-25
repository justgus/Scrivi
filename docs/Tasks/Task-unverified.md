# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0622 — `[Cross]` One version on every platform: `1.0.<commits>`, with the commit hash

**Epic:** [EP-054] (S0; used by [EP-055]'s TestFlight builds and [EP-056]'s `.deb`) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-24) — ✅
**approved** (user, 2026-10-24: *"T-0622 approved"*) · **Sprint:** [SP-193] (🟡 Active) · **Date Requested:** 2026-10-24 (user: *"version 1.0.<some-build-indicator-we-can-manage-as-
development-continues>. Can github do something like that? Can we do a checkin and put the github rev hash at the end of
the version number?"*)

**Today:** the Mac is `MARKETING_VERSION = 1.0`, `CFBundleVersion = 1` (Help ▸ About shows "Version 1.0 (1)"). Linux has
its own counter, `SCRIVI_BUILD_NUMBER 79` in `platforms/linux/generated/ScriviBuildStamp.hpp`, bumped and committed by
`bump-build-stamp.sh` from `~/.scrivi-rig-build-number`.

**Proposal (✅ approved 2026-10-24):**
- **The build indicator = the number of commits on `main`** (`git rev-list --count HEAD`; 296 today). It is computed by
  git itself, increases with every commit, is the same number on every machine for the same commit, and needs no counter
  file. ⚠️ It holds only while `main` is never rewritten (no force-push).
- **Version `1.0.<count>`**, with Apple's build number = `<count>`. ⚠️ Apple accepts **only numbers** in both fields
  (`CFBundleShortVersionString`, `CFBundleVersion`), so the hash cannot go INTO the version on Apple.
- **The hash goes beside it:** Help ▸ About and `scrivi_linux --version` show `1.0.296 (a1b2c3d)`, plus `-dirty` when
  the build has uncommitted changes. Every bug report then names the exact commit.
- ⚠️ **Not checked in:** a commit cannot contain its own hash (writing it changes the hash). So both numbers are read
  from git **at build time**: an Xcode build phase writes them into the built Info.plist, and CMake does the same for
  Linux. The source tree never changes.
- **GitHub's part:** none is needed; git computes it locally. GitHub Releases can host the `.deb` (tagged
  `v1.0.<count>`). ⚠️ A GitHub Actions build must fetch the full history (`fetch-depth: 0`), or the count is 1.
- ⚠️ **Retires the Linux counter:** `bump-build-stamp.sh` and the counter file go, and the rig shows the count + hash of
  the commit it pulled, which proves exactly what it runs (the problem the counter was made for, solved by git).
- ⚠️ TestFlight needs a NEW build number for every upload, so an upload is always of a new commit.

**Implemented (2026-10-24, [SP-193]):**
- `VERSION` (repo root, `1.0`): the one base on every platform.
- **Apple:** `scripts/stamp-version.sh`, a Run Script phase at the end of `ScriviApp` and `ScriviApp-iOS`
  (`C_VERSION_PHASE`, `D_VERSION_PHASE`). It sets `CFBundleShortVersionString` = `1.0.<count>`, `CFBundleVersion` =
  `<count>` and `ScriviCommit` = `<hash>[-dirty]` in the BUILT plist. ⚠️ The phase names the built plist as its input:
  that orders it after Xcode writes the plist (it ran first and was overwritten until it did). An ARCHIVE from a dirty
  tree is refused (SP-193 Q2). Help ▸ About shows `Version 1.0.296 (a566a30)`, selectable.
- **Linux:** `platforms/linux/cmake/ScriviVersion.cmake`, run by the `scrivi_version` custom target on every build. The
  header goes in the BUILD tree as a declared BYPRODUCT, rewritten only when the numbers change. `--version` and
  Help ▸ About read `Scrivi (Linux) 1.0.296 (a566a30), Qt …`. `rig-build.sh` checks the binary names the pushed commit;
  `deploy-to-rig.sh` ships the Mac's numbers in `.scrivi-version-override` (git-ignored) and checks for them.
- **Retired:** `bump-build-stamp.sh`, `platforms/linux/generated/ScriviBuildStamp.hpp`, the `~/.scrivi-rig-build-number`
  counter, `rig-build.sh --no-bump`. The rig doc §4 is rewritten.
- **Checked:** Mac and iOS (simulator) builds stamp `1.0.296 (a566a30-dirty)` with valid signatures, also after a
  version-only change (Xcode re-signs). The archive refusal runs (exit 1 on a dirty tree). The Linux script on the Mac
  gives the same numbers and does not rewrite an unchanged header. In Docker (no `.git`): `1.0.0 (unknown)`; `VERSION`
  changed → `1.7.0` in the SAME build (the 2026-08-30 failure #1 avoided); a second build recompiles nothing.


---

## 🟠 T-0616 — `[Cross]` EP-053 S3: structural cut, Backspace / Delete over scenes, and their undo

**Epic:** [EP-053] (S3) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-22) — ⏳ Mac and rig live passes ([SP-188] AC11) · **Sprint:** [SP-188] (🟡 Active) →
[`../Sprints/Sprint-SP-188.md`](../Sprints/Sprint-SP-188.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** on the Mac and on Linux, a cut, buffer cut, or Backspace / Delete across scenes removes the structure (design R4,
§4.3, Q2, Q4) through one `scrivi_range_cut` off the main thread, the delta applied, no reload; undo is a range paste
(`rangeCut` node), redo the cut. The core balances the cut's join and returns the markers it added, so the undo is exact.
The clipboard (or slot) holds the cut's returned Range with its sidecars; a selection crossing a divider takes that scene
whole; Backspace over one scene divider merges; paste over a cross-scene selection is the cut, then the paste. Old
`structuredCut` nodes and [I-0270]'s grouped deletes still undo. Retires both platforms' `deleteAcrossScenes`.

**Done (2026-10-22):** ScriviCore (`RangeJoin`, the balanced join in `cut()`, applied and checked in `paste()`); the Mac
(`selectionRun`, `cutAcrossScenes(_:keeping:then:)`) and Linux (`ManuscriptEditor::selectionRun`,
`EditorShell::cutAcrossScenes`) — every cross-scene cut, Backspace / Delete and paste-over one range call off the main
thread, the delta applied, undone in one step; `deleteAcrossScenes` retired on both. ctest 763 (Mac) / 767 (Linux),
interop 311, Linux smokes 38; mutations core 6, Mac 8, Linux 7, all caught.

---

*Last Updated: 2026-10-25 (**T-0622**: ✅ Mac half passed (user: About shows the version and hash); ⏳ the rig pass.)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** ([SP-193]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-24 (**T-0620 → ✅ Verified** (user) and archived → `Verified/Task-verified-0620.md`, with [SP-192]'s close.)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** ([SP-192]); ⏳ Mac live pass.)*

*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** ([SP-188]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-22 (**T-0612 → ✅ Verified** (user) and archived → `Verified/Task-verified-0612.md`, with [SP-185]'s close.)*

*Last Updated: 2026-10-22 (**T-0619 → ✅ Verified** (user) and archived → `Verified/Task-verified-0619.md`, with [SP-191]'s close.)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** ([SP-191]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-21 (**T-0605, T-0607, T-0608, T-0609, T-0613, T-0615 → ✅ Verified** (user: Mac + rig live passes, build 75) and archived → `Verified/Task-verified-XXXX.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** ([SP-187]); ⏳ live passes.)*

*Last Updated: 2026-10-20 (**T-0614 → ✅ Verified** (user) and archived → `Verified/Task-verified-0614.md`, with SP-186's close.)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** ([SP-182]); ⏳ rig live pass.)*

