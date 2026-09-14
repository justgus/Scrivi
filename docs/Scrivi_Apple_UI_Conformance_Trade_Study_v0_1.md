# Scrivi — Apple UI Conformance Trade Study v0.1

**Status:** 🟡 **DRAFT — for ruling.** ⚠️ **NOT an approved design document.**
**Date:** 2026-09-14
**Codebase:** `[Apple]` — `Scrivi/Views/`, `Scrivi/App/`
**Occasioned by:** ⚠️ **[I-0203]** (window chrome vanishes while the world-warning banner is up)
and ⚠️ **[I-0205]** (the banner itself). ✅ **Both are findings WITHIN this study, not its scope.**
**Scope ruled by the user 2026-09-14:** ✅ **the WHOLE Apple UI surface**, ⚠️ **not chrome alone.**

---

## 0. ⚠️ READ THIS FIRST — what this document is allowed to claim

⚠️ **The author's model knowledge cutoff is May 2026. macOS 27 is current.** ⚠️ **This document may
therefore NOT assert macOS 27 behaviour from memory** — ✅ **that is exactly how an earlier
`.scrollEdgeEffectStyle` claim was asserted wrongly in [SP-133].**

✅ **SOURCING RULE, USER-RULED 2026-09-14: every API or guideline claim carries a fetched URL.**
⚠️ **Anything not sourced is marked** ⛔ **UNVERIFIED** ⚠️ **and MUST NOT be implemented from.**

✅ **Apple's doc pages are JS-rendered and return only a title to a plain fetch.** ✅ **The reliable
route is the JSON that backs them** — `https://developer.apple.com/tutorials/data/documentation/<path>.json`
— ⚠️ **and that is how every API fact below was obtained.** ✅ **Recorded so the next reader does not
re-discover it.**

### ⚠️ One correction to the [SP-133] record, stated plainly

⚠️ **`NSScrollEdgeEffectStyle` WAS earlier treated as a fabricated API.** ✅ **IT IS REAL:**
**macOS 26.1+** ([source](https://developer.apple.com/documentation/appkit/nsscrolledgeeffectstyle)),
✅ **with the SwiftUI modifier `scrollEdgeEffectStyle(_:for:)` at macOS 26.0+**
([source](https://developer.apple.com/documentation/swiftui/view/scrolledgeeffectstyle(_:for:))).
⚠️ **What was wrong earlier was the SPELLING AND THE PLACE IT WAS APPLIED, not the API's existence.**
✅ **The lesson holds anyway — it was asserted before it was sourced.**

---

## 1. The question this study exists to answer

⚠️ **Scrivi's Apple UI was built surface by surface, each solving its own problem.** ⚠️ **It has never
been assessed AS A WHOLE against how macOS expects an app of this shape to be assembled.**

✅ **[I-0203] is what made that gap visible:** ⚠️ **a warning banner appearing caused FOUR unrelated
pieces of window chrome to disappear at once.** ⚠️ **Four elements restored by ONE dismissal is not
four bugs** — ✅ **it is one structural assumption being wrong.**

⚠️ **The question is therefore NOT "how do we fix the banner".** ✅ **It is: WHERE ELSE is the same
assumption made, and what does macOS 27 actually expect instead?**

---

## 2. ✅ What macOS 26/27 actually specifies — SOURCED

### 2.1 The scroll edge effect is AUTOMATIC and it is the thing misfiring

✅ **SOURCED** — [WWDC25 §310, *Build an AppKit app with the new design*](https://developer.apple.com/videos/play/wwdc2025/310/):

> The scroll edge effect is applied **automatically underneath toolbar items, titlebar accessories,
> and a new type of accessory, split item accessories**. The scroll view varies the size and shape of
> the effect based on the content floating above it.

⚠️ **THREE CONSEQUENCES, and they are the spine of this study:**

1. ✅ **The effect lives INSIDE `NSScrollView`.** ⚠️ **It is not painted by the titlebar; the titlebar
   causes a scroll view to paint it.** ✅ **So "which scroll view" is a real question AppKit must answer.**
2. ✅ **It is AUTOMATIC.** ⚠️ **An app does not opt in — ✅ which means an app CANNOT opt out by
   not adopting it.** ⚠️ **A non-conforming window still gets the machinery, aimed wherever it lands.**
3. ✅ **It adapts "as floating elements come and go".** ⚠️ **That is [I-0203]'s trigger EXACTLY** —
   ✅ **the banner appearing/disappearing changes the floating set and forces a RE-RESOLUTION.**

### 2.2 macOS 27 changed how `automatic` resolves — ⚠️ AND SCRIVI IS IN THE AFFECTED CASE

✅ **SOURCED** — [WWDC26 §289, *Modernize your AppKit app*](https://developer.apple.com/videos/play/wwdc2026/289/):

> The automatic `NSScrollEdgeEffectStyle` resolves to a **hard-edge effect, when there is free-floating
> text, like the window title in the title bar**.

⚠️ **"FREE-FLOATING TEXT … LIKE THE WINDOW TITLE" IS A LITERAL DESCRIPTION OF SCRIVI'S EDITOR WINDOW.**
✅ **§3 establishes that Scrivi's project window has a title and NO toolbar to host it.**
⚠️ **So macOS 27 resolves Scrivi's edge effect to the HARD style — the opaque backing** — ✅ **which is
consistent with the user observing a *"tall TRANSLUCENT RECTANGLE"* rather than a soft fade.**

⚠️ **This is presented by Apple as an automatic benefit for apps that need no rebuild.** ⚠️ **For a
window assembled as Scrivi's is, it is instead a VISIBLE ARTEFACT.** ✅ **The behaviour is correct;
the window it is being applied to is not.**

### 2.3 The APIs that exist for exactly Scrivi's problem

| API | Availability | ✅ Source |
| --- | ------------ | -------- |
| `NSScrollEdgeEffectStyle` (`.automatic` / `.hard` / `.soft`) | **macOS 26.1+** | [JSON](https://developer.apple.com/documentation/appkit/nsscrolledgeeffectstyle) |
| SwiftUI `scrollEdgeEffectStyle(_:for:)` | **macOS 26.0+**, iOS/iPadOS/tvOS/watchOS 26.0+ | [JSON](https://developer.apple.com/documentation/swiftui/view/scrolledgeeffectstyle(_:for:)) |
| SwiftUI `safeAreaBar(edge:alignment:spacing:content:)` | **macOS 26.0+**, all platforms 26.0+ | [JSON](https://developer.apple.com/documentation/swiftui/view/safeareabar(edge:alignment:spacing:content:)) |
| `NSSplitViewItemAccessoryViewController` | ⚠️ **macOS 26.0+** | [JSON](https://developer.apple.com/documentation/appkit/nssplitviewitemaccessoryviewcontroller) |
| `NSSplitViewItem.addBottomAlignedAccessoryViewController(_:)` | macOS 10.10+ (the *accessory class* is 26.0+) | [JSON](https://developer.apple.com/documentation/appkit/nssplitviewitem) |
| `NSSplitViewItem.automaticallyAdjustsSafeAreaInsets` | macOS 10.10+, ⚠️ **defaults to `NO`** | [JSON](https://developer.apple.com/documentation/appkit/nssplitviewitem) |

✅ **`safeAreaBar` is documented as differing from `safeAreaInset` in PRECISELY the relevant way:**

> Unlike `safeAreaInset`, `safeAreaBar` **additionally extends the edge effect of any scroll views**
> affected by the inset safe area.

✅ **`NSSplitViewItemAccessoryViewController` carries `preferredScrollEdgeEffectStyle` and
`automaticallyAppliesContentInsets` (default `true`).** ⚠️ **It is the API whose entire purpose is
"attach a bar to ONE split pane and have the edge effect account for it".**

⚠️ **Scrivi uses NONE of these five.** ✅ **That is the finding, not an incidental detail.**

---

## 3. ✅ What Scrivi actually does — READ IN THE CODE

### 3.1 The project window is hand-assembled and has no toolbar

✅ **`ProjectWindowManager.swift:88-93`** — a bare `NSHostingView` in a `.titled` window:

```swift
window = NSWindow(
    contentRect: NSRect(x: 0, y: 0, width: 1100, height: 700),
    styleMask: [.titled, .closable, .miniaturizable, .resizable],
    backing: .buffered, defer: false)
window.contentView = NSHostingView(rootView: root)
```

⚠️ **NO `NSToolbar`. NO `titlebarAppearsTransparent`. NO `NSSplitViewController`. NO titlebar accessory.**
✅ **Verified by grep across `Scrivi/Views` + `Scrivi/App`: `NSToolbar` appears ZERO times.**

✅ **`.toolbar` appears on macOS in exactly TWO places, BOTH sheets** — `ProjectSettingsSheet.swift:102`
and the navigator's rename sheet (`SceneNavigatorView.swift:734`). ⚠️ **The editor window has none.**

### 3.2 SwiftUI is asked for a title that has nowhere to go

✅ **`EditorView.swift:229-231`:**

```swift
manuscriptDetail
    .navigationTitle(projectTitle)
    .navigationSubtitle(prefs.projectSubtitle)
```

⚠️ **`.navigationTitle` on macOS expects a titlebar/toolbar to render into.** ✅ **§3.1 establishes
there is none.** ⚠️ **This is why the user sees a stray *"The France of Alexandre Dumas"* band sitting
ATOP the manuscript: it is the navigation title, rendering as in-content chrome because it has no
chrome to occupy.**

⚠️ **Note the placement:** ✅ **the title is applied to `manuscriptDetail` — THE SAME VIEW THE BANNER
LIVES INSIDE.** ⚠️ **Title and banner are siblings in one subtree, which is why one moves when the
other appears.**

### 3.3 The banner is a greedy `VStack` sibling, not an accessory

✅ **`EditorView.swift:283-302`** — `ManuscriptTextView` takes `.frame(maxWidth: .infinity, maxHeight:
.infinity)` and the banner is appended BELOW it in the same `VStack`:

```swift
VStack(spacing: 0) {
    ManuscriptTextView(...)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    if session.worldWarningVisible, session.worldWarning.isVisible {
        WorldWarningView(model: session.worldWarning) { session.showWorlds = true }
    }
    ...
}
```

⚠️ **This is the `safeAreaInset`-shaped mistake in its most basic form** — ✅ **a plain sibling, which
does not even inset the safe area, let alone extend the scroll edge effect (§2.3).**
✅ **So the banner changes the scroll view's environment WITHOUT telling the edge-effect machinery.**

⚠️ **NOTE — an earlier hypothesis is DISPROVEN and stays disproven:** ⚠️ **this is NOT a SwiftUI
over-commit (greedy sibling squeezing chrome out).** ✅ **[I-0203]'s captured hierarchy shows
`NSToolbarView` and `NSTitlebarView` PRESENT AND IDENTICAL in both states.** ✅ **Nothing is squeezed;
the edge-effect BINDING changes.**

### 3.4 ⚠️ TWO sibling scroll views, one titlebar — the ambiguity, now named

✅ **The editor's `NavigationSplitView` (`EditorView.swift:211-232`) has a scroll view on BOTH sides:**

| pane | scroll view | ✅ site |
| ---- | ----------- | ------ |
| sidebar | `List(selection:)` → `NSScrollView` | `SceneNavigatorView.swift:104` |
| detail | explicit `NSScrollView` | `ManuscriptTextView.swift:126` |

⚠️ **[I-0203]'s capture shows TWO `NSScrollViewMirrorView` + `NSPortalView` + `_NSPortalLayerBackedView`
triples while the banner is up, and ONE triple + an `NSScrollPocket` when it is dismissed.**
✅ **A mirror triple is a live portal of a scroll view painted behind the titlebar.**
⚠️ **TWO triples = AppKit mirroring TWO scroll views into ONE titlebar.**

✅ **THE TWO CANDIDATES ARE NOW IDENTIFIED STRUCTURALLY** — ⚠️ **the navigator `List` and the
manuscript `NSScrollView`, both direct descendants of the same split view.**

⛔ **NOT YET PROVEN.** ⚠️ **[I-0203] requires these be named BY ADDRESS in a live capture, not
inferred.** ⚠️ **Three code-read diagnoses were wrong earlier in this same investigation; ✅ [I-0204]'s
real cause was found by `sample`, not by reading.** ✅ **§7 carries this as a REQUIRED verification step
and this study does NOT upgrade [I-0203]'s status.**

### 3.5 ⚠️ The same assumption appears elsewhere — this is a CLASS, not one bug

✅ **The Scene Inspector (`SceneInspectorView.swift:39-52`) repeats it twice over:**

```swift
HStack(spacing: 0) {
    resizeHandle                    // ⚠️ hand-rolled splitter, not a split view
    VStack(spacing: 0) {
        Divider()
        selectedTabContent.frame(maxWidth: .infinity, maxHeight: .infinity)
        Divider()
        tabBar                      // ⚠️ hand-built bottom bar, not an accessory
    }
}
.frame(width: paneWidth)
```

⚠️ **`tabBar` is exactly what `NSSplitViewItemAccessoryViewController` /
`safeAreaBar(edge: .bottom)` are for** (§2.3). ⚠️ **`resizeHandle` + `.frame(width:)` re-implements a
split view divider.** ✅ **AND [I-0203] reports the Inspector's TAB BAR is one of the four elements that
vanishes with the banner** — ⚠️ **consistent with it being a hand-built bar in an un-inset safe area.**

✅ **The Timeline strip — the third vanishing element — sits in the SAME `VStack`** (macOS branch, `EditorView.swift:322`, the `#else` arm of the `#if os(iOS)` at `:304`). ✅ **Verified, not inferred.**

⚠️ **So all four [I-0203] symptoms trace to ONE assumption: bars are `VStack` siblings.**

---

## 4. ✅ Findings

| # | Finding | Severity | Evidence |
| - | ------- | -------- | -------- |
| **F1** | ⚠️ **The project window has NO `NSToolbar`, yet asks SwiftUI for `.navigationTitle`/`.navigationSubtitle`.** ✅ **The title renders as an in-content band** | ⚠️ **HIGH** | ✅ §3.1, §3.2 — read |
| **F2** | ⚠️ **macOS 27 resolves `automatic` to the HARD edge style when free-floating title text is present** — ✅ **Scrivi's exact case** | ⚠️ **HIGH** | ✅ §2.2 — [WWDC26 §289](https://developer.apple.com/videos/play/wwdc2026/289/) |
| **F3** | ⚠️ **Bars are `VStack` siblings throughout** (banner, inspector tab bar, timeline) — ✅ **never `safeAreaBar` or a split item accessory**, ⚠️ **so they never inset the safe area or extend the edge effect** | ⚠️ **HIGH** | ✅ §3.3, §3.5 — read |
| **F4** | ⚠️ **Two sibling scroll views under one titlebar with no declared binding** — ✅ **the structural basis of the ambiguity** | ⚠️ **HIGH** | ✅ §3.4 — ⛔ **address-level proof OUTSTANDING** |
| **F5** | ⚠️ **The Scene Inspector hand-rolls a split divider** (`resizeHandle` + `.frame(width:)`) | **Medium** | ✅ §3.5 — read |
| **F6** | ⚠️ **`automaticallyAdjustsSafeAreaInsets` defaults to `NO`** and is never set | **Medium** | ✅ §2.3 — [JSON](https://developer.apple.com/documentation/appkit/nssplitviewitem) |
| **F7** | ⚠️ **[I-0205]: the world-unavailable banner is shown for a condition that may be NORMAL** | **Medium** | ⛔ **NOT diagnosed — see §6** |

---

## 5. Options

### ⚠️ Option A — Suppress the symptom
✅ **Set `.scrollEdgeEffectStyle(.soft, for: .top)` or `scrollEdgeEffectHidden` on the detail.**
✅ **Cheap; probably removes the visible rectangle.** ⚠️ **Fixes NOTHING structural — F1/F3/F4 all
remain, and the next floating element re-triggers it.** ⚠️ **Also fights an AUTOMATIC behaviour (§2.1),
which historically returns.**

### ✅ Option B — Adopt the bar APIs (RECOMMENDED)
✅ **Convert every bar to the API built for it:** ⚠️ **banner → `safeAreaBar(edge: .bottom)` on the
detail; inspector tab bar → same; timeline → same.** ✅ **Directly addresses F3, and F2/F4 follow
because the safe area and edge effect finally account for the bars.**
✅ **Pure SwiftUI, macOS 26.0+, no AppKit restructuring.** ⚠️ **Does NOT by itself fix F1** — ✅ **the
title still has no toolbar** — ⚠️ **so it pairs with B2.**

### ✅ Option B2 — Give the window a real titlebar target
✅ **Either attach an `NSToolbar` to the project window, or stop asking for `.navigationTitle` and set
`window.title`/`window.subtitle` directly.** ⚠️ **These are OPPOSITE directions and must be RULED, not
chosen silently.** ✅ **F2 says the free-floating-title case is what triggers the hard style — ✅ so
giving the title a home is the targeted fix.**

### ⚠️ Option C — Rebuild on `NSSplitViewController`
✅ **The fully conforming shape: real split items, accessory view controllers, `automaticallyAdjusts
SafeAreaInsets = true`.** ✅ **Addresses every finding including F5/F6.**
⚠️ **It is a rewrite of the editor shell** — ⚠️ **the surface that [SP-133] is mid-migration on, and
the one [I-0132]/[I-0161] show is delicate.** ⚠️ **NOT while TextKit 2 is in flight.**

### ✅ RECOMMENDATION
⚠️ **B + B2 now; ✅ C recorded as the destination, NOT scheduled.** ⚠️ **A only if B is measured
insufficient — ✅ never as the first move.**

---

## 6. ⚠️ [I-0205] is NOT resolved by this study

⚠️ **This study explains the banner's LAYOUT DAMAGE. ✅ It says NOTHING about whether the banner
should exist.** ⚠️ **[I-0205]'s own question stands and must NOT be skipped:**

✅ **(a) WHICH world is unavailable and WHY** — ⚠️ **the `dumas-prose` fixture is on the Desktop, not
the removable rig drive, ✅ so a missing USB world is NOT the obvious explanation;**
✅ **(b) is the banner CORRECT-BUT-UGLY, or FALSE?** ⚠️ **These lead to OPPOSITE fixes.**

⚠️ **Per `project_unlinked_world_objects_are_normal`: an absent world is the EXPECTED condition when
its volume is not mounted.** ⚠️ **If the banner is correct, a writer is punished for a normal state;
✅ if false, a world-availability check is wrong, and THAT is data-facing, not cosmetic.**

---

## 7. ⚠️ Required verification before ANY fix

1. ⛔ **Name the TWO mirrored scroll views BY ADDRESS** in a live capture with the banner up.
   ⚠️ **§3.4 identifies them structurally; that is NOT proof.** ⚠️ **[I-0203] demands address-level
   evidence and this study does not substitute for it.**
2. ⛔ **Confirm the hard-style resolution (F2) on the running app** — ⚠️ **§2.2 is Apple's statement,
   not a measurement of Scrivi.**
3. ⛔ **Diagnose [I-0205] (§6) BEFORE touching the banner.**
4. ⚠️ **Re-test after [SP-133] lands** — ✅ **TextKit 2 changes the manuscript scroll view, which is one
   of the two ambiguity candidates.**

---

## 8. ⛔ Explicitly NOT established

- ⛔ **WHY AppKit picks two scroll views rather than one.** ⚠️ **No Apple source found stating the
  binding rule; WWDC25 §310 and WWDC26 §289 were both checked and neither specifies it.**
- ⛔ **Whether `NSScrollPocket` is public API.** ⚠️ **It appears in captures and in third-party reports
  of an unwanted overlay on Tahoe; ✅ no Apple documentation page was found for it.**
- ⛔ **The HIG's window-structure prose.** ⚠️ **The HIG pages are JS-rendered and the mirror returned
  HTTP 500; ✅ §2 rests on WWDC sessions and API docs instead.**
- ⛔ **iOS/iPadOS/visionOS conformance.** ⚠️ **This study read the macOS branch. ✅ The iOS branch
  (`EditorView.swift:166-195`) DOES carry `.toolbar`, so its findings will differ and it needs its own pass.**
