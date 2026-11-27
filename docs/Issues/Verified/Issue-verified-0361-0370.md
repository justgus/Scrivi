# Verified Issues: I-0361 – I-0370

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0351–I-0360 are in [`Issue-verified-0351-0360.md`](Issue-verified-0351-0360.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0362** | `[ScriviCore]` **A paste that creates a chapter or scene next to a pre-T-0358 lowercase order key fails** — "could not find a free chapter order key"; the editor beeps and nothing is pasted. Found importing into `the-sentinel-of-centauri` (folders `chapter-001`, `chapter-V`, `chapter-k`, `chapter-s`). **Cause:** `util::keyBetween` returned "" whenever a bound was lowercase (`hi > "Z"`, and `lo` above the open top), because T-0358's one-time rebalance that was to convert those keys was never wired in. It hit every Range paste, split and chapter move that needs a key there, not only the import. **Fix:** next to a lowercase key, a caps segment is added under the lower key's first segment (`k` .. `s` → `k.A`), in byte order (`ScriviCore/src/util/OrderKey.cpp` `keyBetween`). No folder is renamed. **Tests:** `OrderKeyTests.cpp` (13 legacy gaps, 40 keys each) and `ManuscriptImportCApiTests.cpp` (an import pasted into a project with a `chapter-k` folder, and cut back). The import test fails on the old `OrderKey.cpp` with the live pass's own error. **Verify:** SP-218 Mac step 3 on that project — ✅ passed (user, 2026-11-14). | High | [SP-218] | ✅ **Verified** (2026-11-14 / 2026-11-15) |
| **I-0363** | `[Linux]` **Import ▸ Manuscript… as a new project never got past the save dialog** — after the file and the folder, Save went grey and nothing else happened; only Cancel worked, and no project was written (nothing in `scrivi.log`, no new `.scrivi` on the VM). **Cause:** not proven. It was the only Linux flow that asks for a new project folder through `QFileDialog::getSaveFileName`; New Project and Import Archive ask for a name, then a folder. **Fix:** the same as Import Archive: a name (`QInputDialog`), then `getExistingDirectory` (`platforms/linux/src/ScriviWindow.cpp`, `ShellController::importManuscript`). The core still refuses a folder already there. **Verify:** SP-218's Linux import as a new project — ✅ passed (user, 2026-11-15). | High | [SP-218] | ✅ **Verified** (2026-11-14 / 2026-11-15) |
| **I-0364** | `[Apple]` **Files does not offer Scrivi for a `.scrivi` project** — a double tap or Open With lists no Scrivi (user, 2026-11-19: *"in the Files App, when I navigate to a project and double tap it (or select \"Open With\" from the context menu, Scrivi is not shown as an app that Files can launch, and that I count as an issue."*). **Cause:** the iOS target declared only the archive type (`Info-iOS.plist`): no project or world type, no `CFBundleDocumentTypes`, no `LSSupportsOpeningDocumentsInPlace`; and `openProjectFile` (`AppEnvironment.swift`), reached from iOS's `.onOpenURL` → `handleDeepLink`, never started the Files URL's scope. **Fix:** the Mac's three document types (roles and ranks unchanged) and its project/world declarations in `Info-iOS.plist`, opening in place; on iOS `openProjectFile` closes the open project (one window, as the iPad's Import) and opens through `openProjectFromPickedURL` (scope, bookmark, repair). Verified in the built app's merged Info.plist. ⛔ **Re-pass (user, 2026-11-19):** a Files tap on a cold launch ALSO restored the last session (*"it also tries to load the last project opened … before or after that completes, the dumas project is loaded … This time the sentinel of centauri was loaded by default"*). **Cause:** `.onOpenURL` and the launch restore ran at once; iOS registered both sessions and whichever load finished last took the screen; the hidden one stayed in the manifest. **Fix (iOS):** `.onOpenURL` sets `launchURL` before awaiting; `restoreOpenProjects` gives way to it, closes a restored load that finishes after a tap (unless it is the tapped project, `samePackage`), and restores one project. | Medium | [SP-224] | ✅ **Verified** (2026-11-19) |
| **I-0365** | `[Apple]` **The Landing's buttons are clipped at a compact width** (folded iPhone Duo) (user, 2026-11-19: *"the launch screen on the iPhone Duo in compact size class shows the horizontal row of buttons as clipped in the compact size class.  I recommend change it to a dynamic layout that goes to vertical buttons when the size class is too narrow to display horizontally."*). **Cause:** a fixed `HStack`, inside `.frame(minWidth: 400)` + 40 pt padding that asked any device for 480 pt (`LandingView.swift`). **Fix:** `ViewThatFits` — the row where it fits, a column where it does not; the minimum width is the Mac's only (it sizes the Welcome window). | Low | [SP-224] | ✅ **Verified** (2026-11-19) |
| **I-0366** | `[Apple]` **A save panel that returns the wrong extension fails with a raw "operation not permitted": the app rewrites the URL to `.inkwyn`, a path the sandbox never granted** (New Project, Import as a New Project) | Low | [SP-244] | ✅ **Verified 2026-11-24** |
| **I-0369** | `[Apple]` **Replace… and Remove delete an image another object still shows** (Choose Existing… lets two objects share one) → [details](#i-0369) | Medium | [SP-244] | ✅ **Verified 2026-11-24** |
| **I-0370** | `[Apple]` **A source's citation fields can't be entered, and New Source… says they can** → [details](#i-0370) | Medium | [SP-244] | ✅ **Verified 2026-11-24** |

*Last Updated: 2026-11-24 (**I-0366, I-0369, I-0370** archived with [SP-244]'s close (user, 2026-11-24: *"close SP-244"*).)*

---

## I-0366: `[Apple]` A save panel returning the wrong extension fails as "operation not permitted"

**Found:** 2026-11-20, [SP-229]'s Mac live pass, step 4 (user: *"When I tapped Create it said "the operation is not permitted"."*).
**What happens:** `NewProjectSheet.choosePath()` (`NewProjectSheet.swift:149–151`) and Import ▸ as a New Project
(`ProjectArchiveFlow.swift:457`) take the save panel's URL and, when its extension is not `inkwyn`, replace the extension. The
sandbox grants write access to **the URL the panel returned**, not to the rewritten one, so the create fails with EPERM and the
writer sees only "the operation is not permitted". The rewrite is I-0092's defensive branch (2026-07-28); it can never succeed in a
sandboxed app, and it stayed hidden because the panel always returned the right extension.
**What triggered it:** after the rename the type IDs stayed (`com.caposoft.scrivi.project`, by [T-0684]'s scope) and the extensions
changed; ten stale `Scrivi.app` builds on the Mac still declared the old `.scrivi`, so the panel enforced `.scrivi`. Fixed on the
Mac by unregistering them ([SP-229] progress log). The same can happen on any Mac where an old build registers again.
**Next:** when the panel's extension is wrong, do not rewrite it: tell the writer the location must end in `.inkwyn` (or reopen
the panel). Check the other panels that name a Scrivi type (`AppEnvironment.swift:755`, `ProjectArchiveFlow.swift:147,584`) for the
same shape.
**Severity:** Low — only a Mac with a stale registration hits it; no tester has one.
**Fixed 2026-11-24 ([SP-244], Q4):** the rewrite is gone. `NSSavePanel.runForInkwynProject()` (`NewProjectSheet.swift`) runs the
panel until it returns a `.inkwyn` URL: otherwise an alert (*"An Inkwyn project's name must end in .inkwyn."*) and the panel
again, with the typed name. Used by New Project (`choosePath`) and Import ▸ as a New Project (`ProjectArchiveFlow.swift`). The
other panels were read: Export Archive and Create World keep the URL as returned (no rewrite), the rest are Open panels.
**Status:** ✅ **Verified 2026-11-24** (user, 2026-11-24: *"Step 6: passes.  Step 7: passes.  Step 8: passes."*) (Mac-only). Archived when [SP-244] closes.

---

## I-0369: `[Apple]` Replace… and Remove delete an image another object still shows

**Found:** 2026-11-22, building [SP-235] B2 on Linux from the Mac's `ObjectImageSection.swift`. Not reproduced on the Mac; read from
the code.
**What:** Choose Existing… points an object at an asset already in the world without copying it, so two objects can share one image
(the Mac's own comment at `ObjectImageSection.swift:294`: *"nothing stops two objects referencing one image"*). But **Replace…**
(`importImage`, `:276`) and **Remove** (`removeImage`, `:321`) call `removeAsset` on the old asset unconditionally, so the other
object loses its image's bytes and shows "This object's image is in a world that is unavailable." — a damaged-looking state for a
deliberate edit, and the image is gone.
**Repro (expected):** give A an image; on B, Choose Existing… → A's image; on B, Remove. A's image is gone.
**How Linux does it ([SP-235]):** `removeAssetIfUnshared` (`platforms/linux/src/ObjectDetailDialog.cpp`) deletes the asset only when
no other object's `imagePath` (from `listObjects`) is the same file, and keeps it when it cannot tell. Tested:
`object_detail_smoke` §5.
**Next:** the same guard on the Mac and iOS.
**Severity:** Medium — data loss from an ordinary edit, but only after Choose Existing… has shared an image.
**Fixed 2026-11-24 ([SP-244]):** Replace… and Remove call `ObjectImageSection.removeAssetIfUnshared`, which deletes the asset only
when no other object's `imageAssetID` (from `listObjects`) is it, and keeps it when the list cannot be read — Linux's guard.
Choose Existing… already never deleted. **Test:** interop `sharedImageIsKept` (two objects share one image; the first Remove
keeps the bytes, the second deletes them).
**Status:** ✅ **Verified 2026-11-24** (user, 2026-11-24: *"Step 6: passes.  Step 7: passes.  Step 8: passes."*) (Mac-only). Archived when [SP-244] closes.

---

## I-0370: `[Apple]` A source's citation fields can't be entered, and New Source… says they can

**Found:** 2026-11-22, planning [SP-236] C4 (Q4, (user, 2026-11-22: *"Q1-Q6 as recommended, implement SP-236"*)). Read from the code.
**What:** the citation popover (`SourcesCard.swift`, `ObjectSourcesSection.swift`) shows Author, Year, Publisher, Page, URL and
Accessed from a source's `attributes` (`CitationField`, `Scrivi/Views/Detail/Citation.swift`), but nothing in the app writes them:
the detail sheet has no field for them, and `git log -S` finds no editor ever. New Source…'s form says *"Author, year and the rest are
edited on the source itself — double-click it once it appears."* — the sheet that opens has no such fields. A writer can create a
source and never record who wrote it.
**How Linux does it ([SP-236]):** shows the fields that are set, read-only, as the Mac; its New Source… form does not repeat the
sentence.
**Next:** a citation section in the detail sheet when the object is a source (Mac and iOS first, then Linux); see also [T-0459]
(`page` belongs on the edge).
**Severity:** Medium — a documented capability with no surface, and the app tells the writer it exists.
**Fixed 2026-11-24 ([SP-244], Q5):** the detail sheet (Mac and iOS, `ObjectDetailSheet.swift`) and Linux's detail dialog
(`ObjectDetailDialog.cpp`) edit a source's Author, Year, Publisher, URL and Accessed, saved per field like the other fields
(a field not edited takes disk's value); Page is shown, not edited ([T-0459]). New Source…'s sentence is now true on the Mac
and is Linux's sentence too. **Tests:** Linux `object_detail_smoke` §10b.
**Status:** ✅ **Verified 2026-11-24** (user, 2026-11-24: *"we alerady verified all this.  These all passed."*). Archived when [SP-244] closes.
