# Verified Task — T-0670

**Sprint:** [SP-209] · **Epic:** [EP-058] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-11** (on ScriviCore CI and the scrivi-vm run, then *"close SP-209 now and make the workflow change"*)
**Archived:** 2026-11-11.

---

## ✅ T-0670 — `[ScriviCore]` EP-058 S1: authorship in the core

**Epic:** [EP-058] (S1; AC1–AC5) · **Status:** ✅ **Verified 2026-11-11** (user) · **Sprint:** [SP-209] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-209.md`](../../Sprints/Closed/Sprint-SP-209.md) · **Date Requested:** 2026-11-10 (user: *"activate EP-058 and plan its first sprint"*)

**What:** the core half of the approved authorship design (v0.2 §4.1, §4.4): the ordered authors of a project and of a world
(`identities/*-personas.json`, read at last), writing as per identity (`*-members.json`), the app-level pen-name menu, the
"unnamed" rule and the prompt it raises on open — five endpoints (`scrivi_get_authors`, `scrivi_set_authors`,
`scrivi_set_writing_as`, `scrivi_get_pen_names`, `scrivi_put_pen_names`) and `authors` / `writingAs` / `authorPrompt` on
`scrivi_open_project`, boundary-tested through `scrivi_*`. Shape per [SP-209] Q1–Q5 (✅ ruled as recommended).

**Built (2026-11-11):** `ScriviCore/src/identity/AuthorStore.{hpp,cpp}`; the five endpoints and the open fields
(`scrivi.h`, `scrivi_c_api.cpp`); `controlledBy` as a list (`ProjectPersonasJson.*`, `ProjectCreator.cpp`). Evidence:
`AuthorshipCApiTests.cpp` 9 cases; macOS ctest 834/834; scrivi-vm ctest 839/839, bridge smoke 73/0; ScriviCore CI green on all four jobs (`b59f4f7`).
