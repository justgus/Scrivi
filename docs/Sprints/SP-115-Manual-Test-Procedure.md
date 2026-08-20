# SP-115 — Manual Test Procedure

**Sprint:** SP-115 (EP-034) · **Date:** 2026-08-20 · **Status of code:** all six Tasks
🟢 *Implemented - Not Verified*

**Purpose:** give you a repeatable way to verify each fix **against its own trigger**. Five Issues means
**five separate verifications** — `feedback_verify_each_half_separately` exists because I-0132 was once
credited on the strength of a different half passing.

---

## ⚠️ Read first — three things that will save you time

1. **Back up the USB rig before Test 1.** `tintagael` + `Eskandar` hold real writing work. Test 1 ejects
   the drive; that is safe, but the backup is cheap insurance.
2. **Quit Scrivi before any `xcodebuild test`.** A live instance blocks the runner with a LaunchServices
   error.
3. ⚠️ **T-0420 has NO app surface — Test 5 is deliberately a command-line test.** Nothing in Scrivi reads
   the `unsupportedWorldFormatVersion` error yet. **This is a real gap, honestly reported**, not an
   oversight in this procedure. It is the same *capability-without-surface* shape EP-034 exists to fix,
   and it belongs on the list for a later sprint. **Do not mark I-0136 Verified on app behaviour** — there
   is none to see.

**What is verifiable in the app:** T-0419 (with the rig), T-0421, T-0423.
**What is verifiable only at the command line:** T-0420, T-0422, and T-0419's automated half.

---

## Test 1 — ⚠️ T-0419 / I-0137 · the High one · **REQUIRES THE REAL RIG**

> **Why this cannot be done with a fixture.** A fixture that supplies a `packagePath` **passes while the
> real rig cannot** — that is exactly how AC24 reached "Verified" with a feature that could never fire.
> **Only the physical drive proves this one.**

**Setup**
1. Back up the USB drive.
2. Plug it in. Launch Scrivi, open the **tintagael** project.
3. Confirm the **Eskandar** world resolves: **Worlds ▸ Manage Worlds…** shows it with no warning text.
4. Put at least one world object (e.g. a character) on a scene, so there is a link to hold pending.

**The test**
5. Close the Worlds sheet. **Eject the USB drive** (Finder eject, not a yank).
6. Watch the editor. A warning strip should appear beneath the timeline area.

**PASS criteria — the wording is the whole point**

| Where | ✅ Expected (after fix) | ❌ Before the fix |
| ----- | ---------------------- | ---------------- |
| Warning strip headline | `1 world is unavailable` | same |
| Warning strip sentence | `"Eskandar" is **on a disconnected volume**. N links are held pending.` | `"Eskandar" is **unavailable**. …` |
| **Worlds ▸ Manage Worlds…** row | `on a disconnected volume` | `unavailable` |

7. ⚠️ **The phrase to look for is “on a disconnected volume.”** That is `unmounted` reaching the writer.
   If you see **“unavailable”** anywhere in step 6, **the fix has not taken effect — FAIL.**
8. **Plug the drive back in.** Without touching anything else, the warning should clear on its own
   (I-0129's mount observer). The world returns to available.

**Also confirm — this is the *"absence is never deletion"* half**
9. While the drive is out, the object cards still **list** their world objects, greyed with a ⚠ badge and
   their real names — **not** blank, **not** removed.
10. After reconnecting, nothing needs repair. No object was rewritten.

---

## Test 2 — T-0421 / I-0139 · the inline editor's exit

> **The defect:** a single click on a title opened an edit mode whose only way out was a red **“Revert.”**
> A writer who had changed nothing was told the exit meant data loss, so she did not take it.

**⚠️ Precondition (Q-b):** the inline editor **still exists and must keep working.** This fix relabels its
exit; it does not remove it. If the editor is gone, that is a FAIL, not a success.

**2a — open and leave, having changed nothing (the actual bug)**
1. Open a project with at least one object on a scene. Show the Scene Inspector ▸ **Worldbuilding** tab.
2. **Single-click an object's title** in a card. The inline editor opens.
3. ✅ **PASS:** the exit button reads **“Done”** in normal (non-destructive) styling.
   ❌ **FAIL:** it reads **“Revert”**, or is red.
4. Hover it. ✅ Tooltip: **“Stop editing. Nothing is changed and nothing is deleted.”**
5. Click **Done**. The editor closes; the object is unchanged and still listed.

**2b — change something, then leave (destructive is correct here)**
6. Click the title again. **Edit the name** (add a character).
7. ✅ **PASS:** the button changes to **“Revert”** and is destructive-styled — *because now there is
   something to lose.*
8. Hover it. ✅ Tooltip: **“Revert your changes and stop editing. The object is not deleted.”**
9. Click **Revert**. The name returns to its original. **The object still exists.**

**2c — a new object**
10. Start creating a new object but type nothing.
    ✅ Exit reads **“Cancel”** (nothing to discard).
11. Type a name. ✅ Exit becomes **“Discard”**, destructive-styled.

**⚠️ Also confirm the three things this fix must NOT have done**
- The editor did **not** become modal — you can still click elsewhere in the app.
- There is **no** new Cancel that skips an unfinished-work prompt (I-0119's route).
- A single click on a title **still opens the editor** exactly as before.

---

## Test 3 — T-0423 / I-0138 · disabled **and explained**

**Setup:** the same ejected-drive state as Test 1, **or** any world made unreachable (rename its
`.scrivworld` folder aside in Finder, then reopen the project).

1. With the world unavailable, find a pending object row (greyed, ⚠ badge) in a card.
2. The **⊖ remove** button on that row is greyed out. **Hover it.**
3. ✅ **PASS:** tooltip reads **“Held pending — this object's world is on a disconnected volume.”**
   (or `…is unavailable.` if the world is unreachable for a non-volume reason).
   ❌ **FAIL:** tooltip reads **“Remove from scene”** — the old, unexplained text.
4. Now hover the ⊖ on a **normal** (non-pending) row.
   ✅ It must still read plainly **“Remove from scene.”**
   ⚠️ **This half matters:** the wording is load-bearing (AC22 — the edge goes, the object stays). If it
   now says "Delete" or carries pending text on a healthy row, that is a FAIL.
5. VoiceOver users: the accessibility label on a pending row appends the same explanation.

---

## Test 4 — T-0422 / I-0135 · corrupt `world.json` (command line)

This one has no app surface by nature — it is a regression test proving the core never guesses.

```bash
cd /Users/justgus/Xcode-Projects/Scrivi
./build/ScriviCore/tests/ScriviCoreTests "[T-0422]"
```

✅ **PASS:** `All tests passed`. The test asserts a corrupt `world.json` yields **`unavailable`, never
`missing`**, and that the file is **neither regenerated nor deleted**.

**Optional live check** — worth doing once, because it is the writer-visible consequence:
1. Quit Scrivi. In Finder, open a `.scrivworld` package and replace `world.json` with garbage text.
2. Reopen the project.
3. ✅ The world reports **unavailable** and offers **Locate…**.
   ❌ **FAIL if it says “missing”** — that word invites you to restore from backup over an intact world
   whose single file just needs repair.
4. ✅ Confirm your garbage `world.json` is **still there, byte for byte.** The app must not have quietly
   rewritten it.
5. Restore the original file.

---

## Test 5 — ⚠️ T-0420 / I-0136 · newer `formatVersion` · **NO APP SURFACE**

> ⚠️ **Read the warning at the top of this document.** Nothing in Scrivi surfaces this error today. The
> fix is real and tested at the core; **the writer-facing half does not exist.** Verify the core behaviour
> here, and treat the missing surface as owed work.

```bash
cd /Users/justgus/Xcode-Projects/Scrivi
./build/ScriviCore/tests/ScriviCoreTests "[T-0420]"
```

✅ **PASS:** `All tests passed`. Asserts that a `world.json` declaring `formatVersion: 99`:
- is refused with **`unsupportedVersion`** (not `parseError`, not `validationError` — *"too new" is not
  "damaged"*, and callers must tell them apart);
- carries detail **`unsupportedWorldFormatVersion`**;
- ⚠️ **resolves to `unavailable`, NOT `missing`** — the package is plainly there.

**Optional live check:**
1. Quit Scrivi. Edit a `.scrivworld`'s `world.json`, change `"formatVersion": 1` to `99`.
2. Reopen the project. ✅ The world shows **unavailable** (not "missing"), and no file is rewritten.
3. ⚠️ **Expected shortfall:** you will **not** be told *why* — no "update Scrivi to open this world"
   message exists. **That is the known gap.**
4. Restore `formatVersion: 1`.

---

## Test 6 — T-0424 · the two filed Issues (documentation check)

T-0424 **files, it does not fix.** Verify the filing:

1. `docs/Issues/Issue-active.md` contains **I-0140** and **I-0141**, both 🔴 Open, both assigned to
   **SP-116**.
2. `docs/Issues/Issue-Documentation.md` lists both; next available Issue reads **I-0142**.
3. ✅ Confirm nothing in `ObjectCard.swift:46` or `scrivi.h:97-99` was actually changed —
   **filing, not fixing, was the instruction.**

```bash
git diff --stat Scrivi/Views/Inspector/ObjectCard.swift ScriviCore/include/scrivi/scrivi.h
```
The only changes in those files should be T-0421/T-0423's edits and the T-0419 header documentation —
**no kind-list changes.**

---

## Regression sweep — run before signing anything off

```bash
cd /Users/justgus/Xcode-Projects/Scrivi

# 1. Core suite — expect 524/524
ctest --test-dir build

# 2. ⚠️ Quit Scrivi FIRST, then interop — expect 103 tests in 10 suites
pgrep -x Scrivi && echo "QUIT SCRIVI FIRST" || \
  xcodebuild -scheme ScriviApp -destination 'platform=macOS' test 2>&1 | grep "Test run with"

# 3. Just this sprint's new tests — expect 4 test cases
./build/ScriviCore/tests/ScriviCoreTests "[SP-115]"
```

| Suite | Expected |
| ----- | -------- |
| `ctest` | **524/524** (was 520) |
| interop | **103 tests in 10 suites** (was 99) |
| app build | **BUILD SUCCEEDED** |

---

## Sign-off sheet

⚠️ **Only you can mark these Verified.** Claude has marked them *Resolved / Implemented - Not Verified*.

| Test | Issue | Task | Verifiable in app? | Result |
| ---- | ----- | ---- | ------------------ | ------ |
| 1 | **I-0137** | T-0419 | ✅ **yes — requires the USB rig** | ☐ Pass ☐ Fail |
| 2 | I-0139 | T-0421 | ✅ yes | ☐ Pass ☐ Fail |
| 3 | I-0138 | T-0423 | ✅ yes | ☐ Pass ☐ Fail |
| 4 | I-0135 | T-0422 | ⚠️ test + optional live | ☐ Pass ☐ Fail |
| 5 | I-0136 | T-0420 | ❌ **no surface — core only** | ☐ Pass ☐ Fail |
| 6 | — | T-0424 | 📄 docs check | ☐ Pass ☐ Fail |

⚠️ **Test 1 is the one that matters most and the one most easily faked.** It is the only test in this
sprint that a passing suite genuinely cannot substitute for.

---

*If any test fails, tell me which step and what you saw — the expected strings above are quoted exactly
from the code, so a mismatch localizes the problem immediately.*
