import SwiftUI

// T-0361 — the Scene Inspector shell: bottom tabs + resizable pane.
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §4.7, §C3.
//
// Replaces the SP-090 placeholder (a single segmented tab over a stub "Scene Entities"
// body at a fixed 280pt). Tabs are Writing | Worldbuilding | Properties, defaulting to
// Writing; the selection persists at PROJECT level and does NOT follow the scene —
// switching scenes reloads the current tab's cards for the new scene.

struct SceneInspectorView: View {

    let sceneID: String?
    let projectRootPath: String?
    let engine: ScriviEngine
    let allSceneIDs: [String]
    let history: HistoryCapture?
    let caretByteOffset: Int?
    /// I-0128: world availability revision, so cards refresh on reconnect.
    var worldRevision: Int = 0
    /// I-0155 — bumped by a Detail Sheet save so cards re-read.
    var objectRevision: Int = 0
    /// I-0160 — a card changed an object; the host refreshes the Detail Sheet.
    var onObjectChanged: (() -> Void)? = nil
    /// Attribution for objects created in a card (T-0388).
    let authorshipRef: AuthorshipRef?
    /// Forwarded to the card stacks so an object card can ask the editor to open
    /// the Detail Sheet (SP-117 T-0438).
    var openObjectDetail: ((String, String, String, String) -> Void)? = nil
    @Bindable var layout: InspectorLayoutStore

    // Pane width is a per-device view preference (unlike the layout itself), so it
    // stays in UserDefaults rather than inspector-layout.json.
    // ⛔ T-0546: `@AppStorage("inspectorPaneWidth")` IS RETIRED. The platform owns the
    // inspector's width now (`.inspector` + `inspectorColumnWidth`, applied in
    // EditorView). ⚠️ The key is still READ ONCE, below, so a writer who had dragged
    // the pane keeps her setting — ⛔ dropping it silently would have reset her.

    private static let minWidth: Double = 220
    private static let maxWidth: Double = 560

    /// The `ideal:` width for `.inspector`, migrated from the retired
    /// `@AppStorage("inspectorPaneWidth")` (T-0546 / Q2).
    ///
    /// ⚠️ READ DIRECTLY FROM `UserDefaults`, not via `@AppStorage`, deliberately: this
    /// is a ONE-WAY migration of a value the platform now owns, not live state. An
    /// `@AppStorage` here would re-establish the app as an owner of the width and
    /// invite the two to drift.
    ///
    /// ⚠️ `UserDefaults.double(forKey:)` returns 0 for a MISSING key, which is not a
    /// width — so an absent or out-of-range value falls back to the 300 the retired
    /// property defaulted to, and anything a writer really set is clamped into the
    /// same 220–560 band the hand-rolled handle enforced.
    static var migratedIdealWidth: Double {
        let stored = UserDefaults.standard.double(forKey: "inspectorPaneWidth")
        guard stored > 0 else { return 300 }
        return min(max(stored, minWidth), maxWidth)
    }

    var body: some View {
        // ⛔ T-0546: the `resizeHandle` that used to lead this view is GONE — the
        // platform draws and drives the inspector's divider now.
        //
        // ⚠️ AND THE WRAPPING `HStack` IS GONE TOO, WHICH IS NOT COSMETIC — IT CRASHED.
        //
        // Leaving it as a single-child wrapper put a flexible container between
        // `.inspector`'s `SplitViewChildController` and content declaring
        // `.frame(maxWidth: .infinity)`. The column asked the content for a size, the
        // content answered "as wide as you like", the controller reported a new
        // min/max back to the hosting view, that invalidated layout, and the cycle
        // repeated. AppKit counts constraint passes and throws once they exceed the
        // view count:
        //
        //   NSGenericException: The window has been marked as needing another Update
        //   Constraints in Window pass, but it has already had more … than there are
        //   views in the window.
        //   … SplitViewChildController.hostingView(_:didUpdateMinSize:maxSize:)
        //
        // ⚠️ It only fired when the writer RESIZED the inspector and then opened the
        // Detail Sheet — two size negotiations in flight at once.
        //
        // ⛔ DO NOT reintroduce a flexible-width wrapper here, and ⛔ do not give this
        // view a width constraint of any kind. The COLUMN owns its width
        // (`inspectorColumnWidth` in EditorView); this view fills whatever it is given.
        VStack(spacing: 0) {
            Divider()

            selectedTabContent
                .frame(maxHeight: .infinity)
        }
        // ⚠️ T-0545 / [I-0203] — THE TAB BAR IS A `safeAreaBar`, NOT A STACK SIBLING.
        //
        // ⛔ It was the last member of the VStack above, so it competed for vertical space
        // with the card content. ✅ As a bar it INSETS that content instead.
        //
        // ⚠️ CONVERTED IN [SP-135] (user ruling Q3) and PRESERVED through [SP-136]'s
        // re-host — ✅ [SP-135] AC5 requires exactly that.
        //
        // ✅ The Divider stays INSIDE the bar: it is the bar's top edge, not a separator
        // between two stack members.
        .safeAreaBar(edge: .bottom) {
            VStack(spacing: 0) {
                Divider()
                tabBar
            }
        }
    }

    // MARK: — Tab content

    @ViewBuilder
    private var selectedTabContent: some View {
        switch layout.document.selectedTab {
        case .writing:
            InspectorCardStackView(stack: .writing,
                                   sceneID: sceneID,
                                   projectRootPath: projectRootPath,
                                   engine: engine,
                                   allSceneIDs: allSceneIDs,
                                   history: history,
                                   caretByteOffset: caretByteOffset,
                                   worldRevision: worldRevision,
                                   objectRevision: objectRevision,
                                   onObjectChanged: onObjectChanged,
                                   authorshipRef: authorshipRef,
                                   openObjectDetail: openObjectDetail,
                                   layout: layout)
        case .worldbuilding:
            InspectorCardStackView(stack: .worldbuilding,
                                   sceneID: sceneID,
                                   projectRootPath: projectRootPath,
                                   engine: engine,
                                   allSceneIDs: allSceneIDs,
                                   history: history,
                                   caretByteOffset: caretByteOffset,
                                   worldRevision: worldRevision,
                                   objectRevision: objectRevision,
                                   onObjectChanged: onObjectChanged,
                                   authorshipRef: authorshipRef,
                                   openObjectDetail: openObjectDetail,
                                   layout: layout)
        case .properties:
            ScenePropertiesView(sceneID: sceneID,
                                projectRootPath: projectRootPath,
                                engine: engine)
        }
    }

    // MARK: — Bottom tab bar

    private var tabBar: some View {
        HStack(spacing: 0) {
            ForEach(InspectorTab.allCases) { tab in
                Button {
                    layout.setSelectedTab(tab)
                } label: {
                    VStack(spacing: 2) {
                        Image(systemName: tab.systemImage)
                            .font(.system(size: 13, weight: .medium))
                        Text(tab.title)
                            .font(.system(size: 10))
                            .lineLimit(1)
                            .minimumScaleFactor(0.8)
                    }
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 6)
                    .contentShape(Rectangle())
                    .foregroundStyle(layout.document.selectedTab == tab ? Color.accentColor : .secondary)
                }
                .buttonStyle(.plain)
                .help(tab.title)
            }
        }
        .padding(.horizontal, 4)
        .background(.bar)
    }

    // MARK: — Resize handle

    // ⛔ T-0546 / [EP-040] AC6: `resizeHandle` WAS HERE and is DELETED.
    //
    // It was a 5pt `Rectangle` with a `Divider` overlay, an `.onHover` that pushed and
    // popped `NSCursor.resizeLeftRight`, and a `DragGesture` that clamped
    // `paneWidth` between 220 and 560 — three pieces of platform behaviour
    // re-implemented by hand, on a pane the platform has been able to present since
    // macOS 14.0.
    //
    // ✅ `.inspector(isPresented:)` + `inspectorColumnWidth(min:ideal:max:)` in
    // EditorView now own the column, its divider, its cursor and its width.
    // ⛔ Do not reintroduce a hand-rolled handle. If the inspector needs a different
    // width range, change the `inspectorColumnWidth` bounds.

}
