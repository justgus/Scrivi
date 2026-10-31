import SwiftUI

// The project toolbar, shared by the Mac window and the iOS editor (SP-199 T-0638). iOS places it where the system does —
// the top on iPad and iPhone, the side on the Duo — with no placement code ([SP-198] R3).
struct ProjectToolbar: ToolbarContent {
    let session: ProjectSession
    let env: AppEnvironment

    // MARK: — The window toolbar (EP-040 SP-134 / T-0543, app-shape study S1+S2)
    //
    // ⚠️ WHY IT IS DECLARED HERE, IN SWIFTUI, AND NOT AS AN AppKit `NSToolbar` (ruled 2026-09-22).
    // The project window IS an AppKit `NSWindow`, so an `NSToolbar` was the obvious-looking
    // choice. ⛔ It is the wrong one: `ScriviApp.swift` records that "AppKit NSWindows don't feed
    // SwiftUI's @FocusedValue", which is why the MENU BAR has to reach the session through
    // `env.frontmostSession`. An `NSToolbar` would need that same workaround for EVERY item — a
    // second hand-maintained path to state this view already holds directly.
    // ✅ Declared here, `session` is simply in scope.
    //
    // ⚠️ EVERY ITEM CALLS AN EXISTING CLOSURE. Nothing here re-implements a verb; the toolbar is a
    // SURFACE for what the menu bar already does (EP-040 AC2), and the two can never disagree
    // because there is one code path.
    var body: some ToolbarContent {
        // ⚠️ GROUPED PER MENU, AND THE GROUPING IS FUNCTIONAL — NOT COSMETIC (user ruling,
        // live pass 2026-09-22).
        //
        // ✅ `ControlGroup` is what lets macOS COLLAPSE a group into a single popup control when
        // the window is too narrow to show its members. ⛔ `ToolbarItemGroup` ALONE does not do
        // that — it is layout adjacency only, which is what the first draft used, and at narrow
        // widths its items would simply drop into the generic overflow menu instead of staying
        // together as "Scene" and "Chapter".
        //
        // ⚠️ SO THE NESTING MATTERS: `ToolbarItemGroup { ControlGroup { … } label: { … } }`.
        // The label is what the collapsed popup is titled with, which is why each one names its
        // MENU — a writer who loses the buttons still finds "Scene" and "Chapter".
        //
        // ⚠️ Worlds and Project Settings are DELIBERATELY FREE-STANDING (user ruling): they are
        // project-level surfaces, not members of a verb family, and collapsing them under a
        // shared label would invent a grouping the menu bar does not have.

        // Scene — mirrors the Scene menu, in its order (ScriviApp.swift:202-218).
        ToolbarItemGroup {
            ControlGroup {
                Button { session.createSceneAction?() } label: {
                    Label("New Scene", systemImage: "plus.rectangle")
                }
                .disabled(session.createSceneAction == nil)
                .help("New Scene (⌘↩)")

                Button { session.mergeSceneAction?() } label: {
                    Label("Merge Scene", systemImage: "arrow.triangle.merge")
                }
                .disabled(session.mergeSceneAction == nil)
                .help("Merge Scene with Previous (⌘⌫)")

                Button { session.sceneStartAction?() } label: {
                    Label("Scene Start", systemImage: "arrow.up.to.line.compact")
                }
                .disabled(session.sceneStartAction == nil)
                .help("Go to Scene Start")

                Button { session.sceneEndAction?() } label: {
                    Label("Scene End", systemImage: "arrow.down.to.line.compact")
                }
                .disabled(session.sceneEndAction == nil)
                .help("Go to Scene End")
            } label: {
                Label("Scene", systemImage: "plus.rectangle")
            }
        }

        // ⛔ [T-0568] NO Manuscript toolbar group — REMOVED by user ruling 2026-09-30: its
        // buttons read as Scene Start/End. ✅ The commands live in the Project menu only,
        // and ⌘↑/⌘↓ already reach the manuscript's ends from the text view.

        // Chapter — mirrors the Chapter menu (ScriviApp.swift:220-231).
        ToolbarItemGroup {
            ControlGroup {
                Button { session.createChapterAction?() } label: {
                    Label("New Chapter", systemImage: "plus.rectangle.on.folder")
                }
                .disabled(session.createChapterAction == nil)
                .help("New Chapter (⇧⌘↩)")

                // ⚠️ EVERY VERB HAS ITS OWN GLYPH (user ruling, live pass 2026-09-22): a writer
                // must not have to read a tooltip to tell Merge Scene from Merge Chapter.
                //
                // ✅ `rectangle.compress.vertical` — two bounded regions collapsing into one.
                // ⚠️ Chosen after RENDERING the candidates, not from their names: `arrow.merge`
                // and `arrow.trianglehead.merge` draw the SAME PICTURE as `arrow.triangle.merge`,
                // and no circled merge symbol exists at all. ⛔ `arrow.trianglehead.branch` points
                // OUTWARD — it means branch, the opposite of merge.
                // ✅ It is arguably the truer metaphor here: a chapter IS a container, and merging
                // collapses two containers into one.
                //
                // ⛔ The first draft used `arrow.triangle.merge.circle`, WHICH DOES NOT EXIST, and
                // it rendered as a BLANK BUTTON with only a tooltip. SF Symbol names are not
                // guessable; every symbol here was verified against `NSImage(systemSymbolName:)`.
                Button { session.mergeChapterAction?() } label: {
                    Label("Merge Chapter", systemImage: "rectangle.compress.vertical")
                }
                .disabled(session.mergeChapterAction == nil)
                .help("Merge Chapter with Previous (⇧⌘⌫)")

                // ✅ `.circle` distinguishes Chapter from Scene's `.compact` variant — the
                // user's "enclosed" metaphor. ⚠️ `.alt` was tried first and REJECTED on render:
                // it is NOT a double line, it draws the same as the plain symbol.
                Button { session.chapterStartAction?() } label: {
                    Label("Chapter Start", systemImage: "arrow.up.to.line.circle")
                }
                .disabled(session.chapterStartAction == nil)
                .help("Go to Chapter Start")

                Button { session.chapterEndAction?() } label: {
                    Label("Chapter End", systemImage: "arrow.down.to.line.circle")
                }
                .disabled(session.chapterEndAction == nil)
                .help("Go to Chapter End")
            } label: {
                Label("Chapter", systemImage: "plus.rectangle.on.folder")
            }
        }

        #if os(macOS)
        // Visibility toggles.
        //
        // ⚠️ THERE IS NO NAVIGATOR BUTTON HERE. The first draft added one and it DUPLICATED the
        // system sidebar control macOS already puts in the top-left — same icon, same behaviour.
        // ✅ The platform's control is the affordance [I-0203] wanted; what was missing was STATE
        // behind it, and T-0543's `columnVisibility` binding supplies that.
        // ⛔ Do not re-add a second one.
        ToolbarItemGroup {
            ControlGroup {
                Toggle(isOn: Bindable(session).inspectorVisible) {
                    Label("Inspector", systemImage: "sidebar.trailing")
                }
                .help("Show or hide the Scene Inspector (⌥⌘I)")

                Toggle(isOn: Bindable(session).timelineVisible) {
                    Label("Timeline", systemImage: "calendar.day.timeline.left")
                }
                .help("Show or hide the Timeline (⌥⌘T)")

                // ⚠️ Buffers visibility is APP-GLOBAL, not per-session: one floating palette
                // follows the frontmost project (ProjectSession.swift:48-50). It binds to
                // AppEnvironment, exactly as the View menu's "Show Buffers" does.
                Toggle(isOn: Bindable(env).buffersPaletteVisible) {
                    Label("Buffers", systemImage: "doc.on.clipboard")
                }
                .help("Show or hide the copy buffers (⌥⌘B)")
            } label: {
                Label("Show", systemImage: "sidebar.trailing")
            }
        }
        #endif

        // Project surfaces — FREE-STANDING, one item each (user ruling).
        ToolbarItem {
            Button { session.showWorlds = true } label: {
                Label("Worlds", systemImage: "globe")
            }
            .help("Manage Worlds…")
        }

        ToolbarItem {
            Button { session.showProjectSettings = true } label: {
                Label("Project Settings", systemImage: "gearshape")
            }
            .help("Project Settings… (⌘,)")
        }

        // ⚠️ DECLARED SLOTS — EP-040 AC7 / AC5. Export and the text-size lens are what this
        // toolbar makes ROOM for; ⛔ BUILDING THEM IS OUT OF SCOPE for this Epic (app-shape §4.2:
        // "they are NEW FEATURES and should not be bundled into a structural restructure").
        //
        //   Document ▸ Export…              — no implementation exists yet
        //   View     ▸ text-size lens       — no implementation exists yet (app-shape §2.3)
    }
}
