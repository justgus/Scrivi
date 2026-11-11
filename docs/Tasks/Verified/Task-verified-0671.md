# Verified Task — T-0671

**Sprint:** [SP-210] · **Epic:** [EP-058] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-11** (user, 2026-11-11: *"8: Scrivi's settings.  The Pad did not ask for access to keychain in step 10.  Other than that Steps 1-10 passed."*)
**Archived:** 2026-11-11.

---

## ✅ T-0671 — `[Apple]` EP-058 S2: authorship on the Mac and iPad

**Epic:** [EP-058] (S2; AC3, AC4, AC6, AC8) · **Status:** ✅ **Verified 2026-11-11** (user) · **Sprint:** [SP-210] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-210.md`](../../Sprints/Closed/Sprint-SP-210.md) · **Date Requested:** 2026-11-11 (user: *"plan EP-058 S2"*)

**What:** the Apple half of the authorship design (v0.2 §4.2, §4.4) over [SP-209]'s endpoints: the engine wrappers; the
author an edit is recorded under built per project from `writingAs` (not the app-global identity); New Project's Author;
Pen Names; Project Settings ▸ Authors with I write as; Worlds ▸ Authors…; the *Who wrote…?* and *Which of these authors
are you?* prompts on open — on the Mac and iPad. Shape per [SP-210] Q1–Q6 (✅ ruled; Q1 amended: Mac Settings window on ⌘,, iPad in-app sheet).

**Built (2026-11-11):** engine wrappers + `Author` / `AuthorsResult` / `AuthorEdit` / `PenNames` and the open's three fields
(`ScriviEngine.swift`, stubs); the window's author from `writingAs` (`ProjectSession.authorshipRef` / `adoptWritingAs`, the 7
`env.authorshipRef` reads moved to the session); New Project's Author (`NewProjectSheet.swift`, `AppEnvironment.createProject`);
`Scrivi/Views/AuthorshipViews.swift` (new) — App Settings ▸ Pen Names, Project Settings ▸ Authors and I write as, Worlds ▸
Authors…, the two prompts; Mac `Settings` scene on ⌘, (Project Settings → ⌥⌘,); iPad Launch toolbar ⚙, the ⋯ menu's Scrivi
Settings, the menu bar's Settings… (⌘,). Evidence: `AuthorshipInteropTests` 7/7; macOS interop 347/347; iOS 19/19; ScriviCore
ctest 834/834; Mac and iOS builds clean (no warnings); stub parity, boundary, ABI guards, TextKit 2, layout and type-source
checks clean. visionOS still fails only on [I-0356] (`EditorView` inspector, predates this).

**Live pass (user, 2026-11-11):** AC7 steps 1–10 passed on the Mac and iPad; the iPad menu bar's ⌘, opens Scrivi's own Settings sheet; the iPad asked *Which of these authors are you?* for the Mac's project with no keychain prompt (iOS asks none: the app's own keychain).
