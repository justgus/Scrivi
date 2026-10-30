---
sprint: SP-198
epic: EP-055
status: Active
platform: Apple
created: 2026-10-29
---

# SP-198 — `[Apple]` [EP-055] **S1**: the manuscript on iPad and iPhone

**Status:** 🟡 **ACTIVE 2026-10-29** (user: *"Activate EP-055, go with 27.1 … Duo simulator only for now. I have an iPhone
and iPad to test on."*).
**Carries:** [T-0632], [T-0633], [T-0634], [T-0635] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-055] (AC1–AC4) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Goal:** a writer opens a real project on an iPad or an iPhone and the manuscript **works properly**: good to read and
good to write on, with the system's default keyboard and layout behaviour (R3).

---

## What reading the code found (2026-10-29)

- The iOS manuscript is a stub (`ManuscriptTextView.swift:3700–3718`). The Mac host is the `NSViewRepresentable` (`:37`),
  its `Coordinator` (`:287`) and `ManuscriptNSTextView` (`:2614`).
- `ManuscriptPresenter` (TextKit 2 delegates), `ManuscriptPresentedMap`, `ManuscriptFind` and `ManuscriptAccessibility`
  are `#if os(macOS)`; their AppKit use is mostly `NSFont` / `NSColor`. `ManuscriptTypography` already guards AppKit with
  `canImport` (`:3,129`).
- The iOS shell already exists: `EditorView`'s `NavigationSplitView` (`EditorView.swift:193`), the document picker
  (`LandingView.swift:84`). ⚠️ `EditorView.swift:406,475` branch on `userInterfaceIdiom`, which Apple's Duo guidance
  says not to do; they are outside the manuscript, so not this Sprint's.
- ✅ The iOS target builds for the Simulator (Xcode 27.2) at the new 27.1 target.

## Rulings

- ✅ **R1 — iOS deployment target 27.1** (user, 2026-10-29; the user set it): `IPHONEOS_DEPLOYMENT_TARGET` on the
  `ScriviApp-iOS` target (`project.pbxproj:1023,1056`); the ScriviCore cross-build follows it (`:644`). CLAUDE.md updated.
- ✅ **R2 — devices:** a real iPad and iPhone for live passes; the Duo in the simulator only (user, 2026-10-29).
- ✅ **R3 — no special handling for the Duo or for keyboards.** *"stay using only the size classes and don't do anything special for iPhone Duo until I identify a need … don't do anything special with regard to keyboards on either the iPhone or iPad as well. The default layout will be sufficient … iOS also detects when a hardware keyboard is in place and acts accordingly."* Layout by size class only; toolbar buttons at their default placement.
- ✅ **R4 — an iOS test target** (user, 2026-10-30: *"We will need to create a test target for iOS."*) → [T-0635].

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **What moves out of the Mac `Coordinator`** so iOS does not re-implement it? | [T-0632] answers it from the code, then the user rules. |

## Acceptance Criteria

- [ ] AC1 — The manuscript host gap audit written and Q1 ruled ([T-0632])
- [ ] AC2 — The shared presentation layer compiles for iOS and the Mac behaves exactly as before: interop tests green and
      the TextKit 2 / type-source checks pass ([T-0633])
- [ ] AC3 — On iPad and iPhone a real project opens to its manuscript, scrolls and reads with the Mac's typography,
      dividers and chapter titles, and typed text autosaves to the scene file ([T-0634])
- [ ] AC4 — An iOS test target that runs on the simulator and covers the manuscript ([T-0635])
- [ ] AC5 — Live passes on the user's iPad and iPhone (and the Duo simulator), on-screen and hardware keyboards, with
      nothing device- or keyboard-specific added (R3)

*Last Updated: 2026-10-30 (R3: no Duo or keyboard special handling; R4: [T-0635] the iOS test target; Q2 withdrawn.)*

*Last Updated: 2026-10-29 (created and activated with EP-055.)*
