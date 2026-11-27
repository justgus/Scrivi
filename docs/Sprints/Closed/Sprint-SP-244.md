---
sprint: SP-244
epic: none
status: Closed
platform: Cross
created: 2026-11-24
---

# SP-244 — `[Cross]` Pre-alpha bug sweep: the bugs a tester would meet first

**Status:** ✅ **CLOSED** 2026-11-24 (user, 2026-11-24: *"close SP-244"*). AC1–AC8 ✅; all seven Issues Verified and archived; I-0323's cause → [I-0375], closed Unreproducible. Was 🟡 **ACTIVE** 2026-11-24 (user, 2026-11-24: *"Q1-Q7 as recommended, implement SP-244"*); Q1–Q7 as recommended. Created 2026-11-24 (user: *"let's start planning the bug sprint"*).
**Epic:** none — a sweep before [SP-228] (EP-055 AC9, TestFlight) archives the alpha.
**Carries (Q1):** [I-0366], [I-0369], [I-0353], [I-0284], [I-0323], [I-0370]; ➕ [I-0374] (folded in 2026-11-24, user).
**Size:** M — six Issues, two of them core changes, three on both platforms.

---

## Goal

The six open defects a TestFlight tester is most likely to meet on their first day are fixed on the Mac and iOS, and on
Linux where they exist there, before [SP-228] archives the alpha.

## What reading the code found (2026-11-24)

Each Issue's record was re-checked against today's code; all six are still as filed.
- **[I-0366]** `NewProjectSheet.swift:149–151` still rewrites the panel's URL to `.inkwyn`, a path the sandbox did not grant.
- **[I-0369]** `ObjectImageSection.swift:276` (Replace…) and `:321` (Remove) still call `removeAsset` unconditionally; the
  file's own comment at `:295` names the shared-image case. Linux's guard is `removeAssetIfUnshared`
  (`platforms/linux/src/ObjectDetailDialog.cpp`, tested in `object_detail_smoke` §5).
- **[I-0370]** `ObjectSourcesSection.swift:194` still tells the writer the citation fields are *"edited on the source itself"*;
  no editor exists. Linux shows them read-only and does not repeat the sentence (`ObjectDetailDialog.cpp:1973`).
- **[I-0353]** the core refuses every paragraph format on an empty line (`scrivi.h:1248`, `paragraph_edit`); both platforms
  beep. Ruled 2026-11-05: every Format command on an empty line applies to what is typed next.
- **[I-0284]** measured 2026-10-08 in `MarkdownBlocks.analyze`: a hard break (`\` + newline) in a paragraph drops all its
  emphasis markers. Linux (md4c) not yet checked.
- **[I-0323]** the cause is not named: an older build read an empty or absent file. `util::parseJson` (`Json.cpp:174`) returns
  `e.what()` as the message.

**Left in the backlog (Q1):** [I-0301] (intermittent; waiting for the `AGAINST WHEEL` log lines, with no cause to fix yet) and
[I-0354] (both platforms, and needs a Spotlight ruling of its own).

## Plan

1. **[I-0366]** — no rewrite: when the panel returns another extension, say the location must end in `.inkwyn` and reopen the
   panel (Q4). Same check at `ProjectArchiveFlow.swift:457`, and the other panels that name a project type
   (`AppEnvironment.swift:755`, `ProjectArchiveFlow.swift:147,584`) read for the same shape.
2. **[I-0369]** — the Linux guard on the Mac and iOS: delete the old asset only when no other object's `imagePath` names it, and
   keep it when that cannot be told. An interop test: two objects share one image, one removes it, the other still reads it.
3. **[I-0353]** (core first, then both platforms) — `paragraph_edit` on an empty line succeeds: a heading or list starts the
   line (`# `, `- `, `1. `), and Body on a body line succeeds with nothing changed (Q2). Boundary tests through `scrivi_*`; the
   Mac, iOS and Linux stop beeping because the core no longer refuses.
4. **[I-0284]** — find why the analyzer drops emphasis after a hard break and fix it; check Linux with the same text and fix it
   there too if it is wrong (Q6). The SP-171 corpus test gains the hard-break case.
5. **[I-0323]** — reproduce first: build the 2026-09-16 commit in a scratch copy (never this tree), open a current project,
   log the path it reads. Then (Q3): the parse error names the file in a plain sentence; and if the cause is a format the old
   build cannot read, the open says the project was saved by a newer version of Inkwyn.
6. **[I-0370]** — the citation fields become editable in the detail sheet when the object is a source: Mac and iOS, and Linux
   in the same Sprint (Q5). New Source…'s sentence then tells the truth.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Scope.** | **The six above.** Leave [I-0301] in the backlog: it needs the next occurrence's log, and a fix without a cause is a guess ([I-0204]'s history). Leave [I-0354] for a Sprint of its own: it touches four Mac stores and the Linux session, and its Spotlight question needs ruling first. | ✅ As recommended (2026-11-24) |
| **Q2** | **[I-0353]: what an empty formatted line stores.** | **The Markdown marker alone** (`# `, `- `, `1. `), as other Markdown editors do. If the writer types nothing and moves on, the empty heading stays in the scene, visible, and can be deleted like any line. **Body** on a line that is already body succeeds silently (no flash). | ✅ As recommended (2026-11-24) |
| **Q3** | **[I-0323]: how far the fix goes.** | **Find the cause first; then (1) always:** the message names the file and reads as a sentence; **(2) only if the cause is format drift:** "This project was saved by a newer version of Inkwyn." If the cause is something else, (2) is not built, and a new Issue is filed if one is owed. | ✅ As recommended (2026-11-24) |
| **Q4** | **[I-0366]: what the writer sees.** | **An alert** — *"An Inkwyn project's name must end in .inkwyn."* — **then the save panel again**, with the name they typed. No rewrite, so nothing the sandbox did not grant is touched. | ✅ As recommended (2026-11-24) |
| **Q5** | **[I-0370]: which fields, and Linux.** | **Author, Year, Publisher, URL and Accessed are editable on the source; Page is not added to the editor** — [T-0459] (ruled 2026-08-24) moves `page` onto the `cites` edge, so an editor for it on the source would be built and then moved. A Page value already stored still shows. **Linux gets the same editor in this Sprint** (Linux adopts the Mac's shape in the same work). | ✅ As recommended (2026-11-24) |
| **Q6** | **[I-0284] on Linux.** | **Check it, and fix it here if Linux is wrong too.** If Linux is right, nothing changes there. | ✅ As recommended (2026-11-24) |
| **Q7** | **Live passes.** | **The Mac, then iPad and iPhone, then Linux on `scrivi-vm`.** A rig pass, if one is wanted, goes to [EP-057] and does not hold the Sprint open. | ✅ As recommended (2026-11-24) |

## Acceptance Criteria

- [x] AC1 — [I-0366]: a save panel that returns another extension shows the alert and reopens; no "operation not permitted".
- [x] AC2 — [I-0369]: removing or replacing a shared image leaves the other object's image intact (interop test + Mac pass).
- [x] AC3 — [I-0353]: Heading 1–3, both lists and Body on an empty line succeed through `scrivi_*` (boundary tests) and on the
      Mac, iOS and Linux, with no beep.
- [x] AC4 — [I-0284]: emphasis after a hard break renders on the Mac and iOS (and Linux, per Q6); the corpus test covers it.
- [x] AC5 — [I-0323]: the cause is named in the Issue; the message names the file; per Q3, the newer-version message.
- [x] AC6 — [I-0370]: a source's citation fields are entered and kept on the Mac, iOS and Linux; New Source…'s text is true.
- [x] AC7 — ctest, the interop tests, the iOS tests and the Linux smokes green; CI green on the push.
- [x] AC8 — the live passes per Q7.

## Progress log

- **2026-11-24** — 🔵 created (user: *"let's start planning the bug sprint"*); ID issued by `next-id.py`. Every Issue re-read
  against the code.
- **2026-11-24** — 🟡 **ACTIVE** (user, 2026-11-24: *"Q1-Q7 as recommended, implement SP-244"*). The six Issues → `Issue-active.md`.
- **2026-11-24** — **Built.** All six → 🟠 Resolved - Not Verified (`Issue-active.md` has each fix):
  - [I-0366] `NSSavePanel.runForInkwynProject()`: alert + panel again, never a rewrite (New Project, Import as a New Project).
  - [I-0369] `ObjectImageSection.removeAssetIfUnshared` (Linux's guard) on Replace… and Remove; interop test.
  - [I-0353] in the core: an empty line takes the marker alone; Body there is `.unchanged` (no flash, no history event).
    ➕ A caret at the very end of a scene was refused too — fixed in the same function; 12 parity-fixture lines re-recorded
    (`why: "I-0353"`).
  - [I-0284] cause measured: after a hard break Apple's parser reports the break's line with running columns;
    `MarkdownBlocks.offset` carries them on. Linux (md4c) was right (Q6: no change).
  - [I-0323] ⚠️ **cause NOT reproduced** (the 2026-09-16 app and core open today's projects; an AppleDouble sidecar too), so
    Q3 (2) is not built; (1) built: a sentence, and the file named on the open path.
  - [I-0370] the citation fields in the Mac/iOS detail sheet and Linux's detail dialog; Page shown only.
  - Tests: ctest 885/885 on the Mac (in `build-coretests/`: Xcode resets `build/` to tests OFF), 890/890 on `scrivi-vm`;
    Mac interop 356/356; iOS 38/38; Linux smokes 50/50 (`object_detail_smoke` §10b new). ⚠️ One parallel ctest run on the
    Mac failed `T-0478` once and passed on re-run ([I-0355], the clock-named temp folders).
    ⏳ AC7: CI on the push · AC8: live passes.
- **2026-11-24** — Mac live pass (user): steps 1–4 and 6–8 ✅ pass. ⚠️ Step 5 was WRONG: it said to TYPE `**two**`, which the
  escape ruling stores as literal asterisks (the Mac and Linux did exactly that) — bold must come from Format ▸ Bold. Re-run
  with the corrected step. The iPad turned typed asterisks INTO formatting → filed [I-0374] (backlog).
- **2026-11-24** — corrected step 5 ✅ on the Mac (user, 2026-11-24: *"fold I-0374 into SP-244.  Step 5-1 to 5-4 pass."*) → [I-0284] ✅ Verified. ➕ [I-0374] folded into this Sprint.
- **2026-11-24** — [I-0374] → 🟠: UIKit's own replacements (`replace(_:withText:)`) were stored unescaped; now escaped
  through the rules. iOS tests 39/39. ⚠️ "a scene typed on iOS is autosaved" failed once and passed on re-run (a fixed 1.6 s wait).
- **2026-11-24** — [I-0374] re-reported on the iPad's hardware keyboard; the device log showed a key reaches the delegate
  with the raw `*` before `insertText`. The delegate now judges typed text escaped (`shouldChangeTyped`). iOS tests 40/40.
- **2026-11-24** — [I-0374] ✅ Verified (user, 2026-11-24: *"Ok this now works on both the iPhone with a software keyboard and the iPad with the hardware keyboard."*).
- **2026-11-24** — iOS and Linux live passes ✅ (user, 2026-11-24: *"we alerady verified all this.  These all passed."*) → [I-0353], [I-0370] ✅ Verified; AC1–AC4, AC6, AC8 met.
- **2026-11-24** — [I-0323] ✅ Verified, as built; its cause filed as [I-0375] and closed **Unreproducible** (user, 2026-11-24: *"I-0323 verified as built, file the cause to unreproducable.  close it on my authority."*) → AC5 met (amended: cause unreproducible).
- **2026-11-25** — AC7 ✅: CI green on `c235d27` (Scrivi Apple CI); the only code since `b9244f9` (all three green) is under `Inkwyn/`, so Core and Linux CI did not run by their path filters. All ACs met.
- **2026-11-24** — ✅ **CLOSED** (user, 2026-11-24: *"close SP-244"*). I-0284, I-0323, I-0353, I-0366, I-0369, I-0370, I-0374 → `docs/Issues/Verified/`.
