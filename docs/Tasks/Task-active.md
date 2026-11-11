# Active Tasks

*Last Updated: 2026-11-11 (**T-0670 → active** with [SP-209]'s activation (EP-058 S1).)*

*Last Updated: 2026-11-10 (**T-0668 → ✅ Verified** ([SP-208] closed) → `Verified/Task-verified-0668.md`.)*

*Last Updated: 2026-11-10 (**T-0668 → active** with [SP-208]'s activation (EP-054 S4).)*

*Last Updated: 2026-11-09 (**T-0666 → ✅ Verified** and archived → `Verified/Task-verified-0666.md` ([SP-207] closed) (user, 2026-11-09: *"I-0357 verified, close SP-207"*))*

*Last Updated: 2026-11-09 (**T-0666 → active** with [SP-207]'s activation (EP-054 S3).)*

*Last Updated: 2026-11-06 (**T-0665 → 🟠** → `Task-unverified.md` ([SP-206] built; the live passes are the user's).)*

*Last Updated: 2026-11-06 (**T-0665 → active** with SP-206's activation (EP-054 S2).)*

*Last Updated: 2026-11-06 (**T-0664 → 🟠** → `Task-unverified.md` ([SP-205] built; core only, no live pass).)*

*Last Updated: 2026-11-06 (**T-0664 → active** with SP-205's activation (EP-054 S1).)*

*Last Updated: 2026-11-05 (**T-0611 → 🟠** → `Task-unverified.md` ([SP-184] built; the live pass is the user's).)*

*Last Updated: 2026-11-05 (**T-0611 → active** with SP-184's activation (EP-048 S5c).)*

*Last Updated: 2026-11-05 (**T-0610 → 🟠** → `Task-unverified.md` ([SP-183] built; the live pass is the user's).)*

*Last Updated: 2026-11-05 (**T-0610 → active** with SP-183's activation (EP-048 S5b).)*

*Last Updated: 2026-11-05 (**T-0662 → 🟠** → `Task-unverified.md` ([SP-204] built; Mac string check and live pass pending).)*

*Last Updated: 2026-11-05 (**T-0662 → active** with SP-204's activation (EP-026 S3).)*

*Last Updated: 2026-11-04 (**T-0658 → 🟠** → `Task-unverified.md` (SP-202 measured and ruled).)*

*Last Updated: 2026-11-04 (**T-0658 filed → active** with SP-202's activation (EP-050 S3).)*

*Last Updated: 2026-11-02 (**T-0646 → 🟠** → `Task-unverified.md`: its rig pass moved to [T-0653] ([EP-057]).)*

*Last Updated: 2026-11-01 (**T-0400, T-0643–T-0645 → 🟠** → `Task-unverified.md`; T-0646 stays active for the live passes.)*

*Last Updated: 2026-11-01 (**T-0400, T-0643–T-0646 → active** with SP-200's activation (EP-052 AC11).)*

*Last Updated: 2026-10-31 (**T-0639–T-0642** filed from SP-199's live pass and implemented on Apple → 🟠 `Task-unverified.md`.)*

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 filed → active** with SP-199's activation (EP-055 S2).)*

---

## 🟡 T-0670 — `[ScriviCore]` EP-058 S1: authorship in the core

**Epic:** [EP-058] (S1; AC1–AC5) · **Status:** 🟡 Active (2026-11-11) · **Sprint:** [SP-209] (🟡 Active) →
[`../Sprints/Sprint-SP-209.md`](../Sprints/Sprint-SP-209.md) · **Date Requested:** 2026-11-10 (user: *"activate EP-058 and plan its first sprint"*)

**What:** the core half of the approved authorship design (v0.2 §4.1, §4.4): the ordered authors of a project and of a world
(`identities/*-personas.json`, read at last), writing as per identity (`*-members.json`), the app-level pen-name menu, the
"unnamed" rule and the prompt it raises on open — five endpoints (`scrivi_get_authors`, `scrivi_set_authors`,
`scrivi_set_writing_as`, `scrivi_get_pen_names`, `scrivi_put_pen_names`) and `authors` / `writingAs` / `authorPrompt` on
`scrivi_open_project`, boundary-tested through `scrivi_*`. Shape per [SP-209] Q1–Q5 (✅ ruled as recommended).

**Built (2026-11-11):** `ScriviCore/src/identity/AuthorStore.{hpp,cpp}`; the five endpoints and the open fields
(`scrivi.h`, `scrivi_c_api.cpp`); `controlledBy` as a list (`ProjectPersonasJson.*`, `ProjectCreator.cpp`). Evidence:
`AuthorshipCApiTests.cpp` 9 cases; macOS ctest 834/834. ⏳ Linux via CI after the push.
